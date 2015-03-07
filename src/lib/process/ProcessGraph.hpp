#pragma once

#include "process/Process.hpp"
#include "process/ProcessGroup.hpp"

#include <glog/logging.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <string>
#include <vector>

class ProcessGraph {
public: // types
    typedef std::vector<Process::Ptr> NodeList;
    typedef NodeList::size_type NodeId;

    struct FdSpec {
        NodeId node_id;
        int fd;

        friend bool operator<(FdSpec const& x, FdSpec const& y) {
            if (x.node_id < y.node_id)
                return true;
            if (y.node_id < x.node_id)
                return false;
            return x.fd < y.fd;
        }

        friend bool operator==(FdSpec const& x, FdSpec const& y) {
            return x.node_id == y.node_id && x.fd == y.fd;
        }
    };


    typedef boost::unordered_set<FdSpec> FdSpecSet;
    typedef boost::unordered_map<FdSpec, FdSpecSet> PipeMap;

public: // functions
    NodeId add(Process::Ptr const& proc) {
        pgroup_.add(proc);
        nodes_.push_back(proc);
        return nodes_.size() - 1;
    }

    NodeId add(std::vector<std::string> const& args) {
        return add(Process::create(args));
    }

    NodeId size() const {
        return nodes_.size();
    }

    Process::Ptr& process(NodeId id) {
        validate_node(id);
        return nodes_[id];
    }

    Process::Ptr const& process(NodeId id) const {
        validate_node(id);
        return nodes_[id];
    }

    void validate_node(NodeId id) const {
        using boost::format;

        if (id >= nodes_.size()) {
            throw std::runtime_error(str(format(
                "Request for invalid node id %1%"
                ) % id));
        }
    }

    void connect(NodeId src, int src_fd, NodeId dst, int dst_fd) {
        validate_node(src);
        validate_node(dst);
        FdSpec a = {src, src_fd};
        FdSpec b = {dst, dst_fd};
        pipes_[a].insert(b);
    }

    void create_pipe(int rwpipe[2]) {
        if (pipe(rwpipe)) {
            throw std::runtime_error("failed to open pipe");
        }

        LOG(INFO) << "Created new pipe. (r, w) fds: ("
            << rwpipe[0] << ", " << rwpipe[1] << ")";
    }

    bool execute() {
        std::vector<FdSpec> broadcasters;

        for (PipeMap::const_iterator iter = pipes_.begin(); iter != pipes_.end(); ++iter) {
            FdSpecSet const& destinations = iter->second;
            if (destinations.size() > 1) {
                broadcasters.push_back(iter->first);
            }
        }

        for (std::size_t i = 0; i < broadcasters.size(); ++i) {
            FdSpec const& src = broadcasters[i];
            PipeMap::iterator iter = pipes_.find(src);
            NodeId tee_id = add(make_fdtee_cmd(3, iter->second.size()));
            FdSpecSet dst_copy = iter->second;
            pipes_.erase(iter);
            connect(src.node_id, src.fd, tee_id, 3);
            int tee_fd = 4;
            for (FdSpecSet::const_iterator dst_iter = dst_copy.begin(); dst_iter != dst_copy.end(); ++dst_iter) {
                connect(tee_id, tee_fd++, dst_iter->node_id, dst_iter->fd);
            }
        }

        for (PipeMap::const_iterator siter = pipes_.begin(); siter != pipes_.end(); ++siter) {
            int rwpipe[2];
            create_pipe(rwpipe);

            pipe_fds_.push_back(rwpipe[0]);
            pipe_fds_.push_back(rwpipe[1]);

            FdSpec const& src = siter->first;
            FdSpecSet const& destinations = siter->second;
            std::size_t n_dst = destinations.size();
            if (n_dst == 1) {
                FdSpec const& dst = *destinations.begin();
                Process::Ptr& src_proc = process(src.node_id);
                Process::Ptr& dst_proc = process(dst.node_id);
                src_proc->fd_map().add_existing_fd(src.fd, rwpipe[1]);
                dst_proc->fd_map().add_existing_fd(dst.fd, rwpipe[0]);

                LOG(INFO) << "piping (proc " << src.node_id << ", fd "
                    << src.fd << " -> (proc " << dst.node_id << ", fd "
                    << dst.fd  << ")";
            }
            else {
                throw std::runtime_error("unsupported number of destinations");
            }
        }

        pgroup_.start();
        for (std::size_t i = 0; i < pipe_fds_.size(); ++i) {
            close(pipe_fds_[i]);
        }

        return pgroup_.finish();
    }

    static std::vector<std::string> make_fdtee_cmd(int read_fd, std::size_t n_dst) {
        std::vector<std::string> args;
        args.push_back("fdtee");
        args.push_back(boost::lexical_cast<std::string>(read_fd));
        for (std::size_t i = 1; i <= n_dst; ++i) {
            args.push_back(boost::lexical_cast<std::string>(read_fd + i));
        }
        return args;
    }

private: // data
    NodeList nodes_;
    ProcessGroup pgroup_;

    std::vector<int> pipe_fds_;
    PipeMap pipes_;
};

inline
std::size_t hash_value(ProcessGraph::FdSpec const& spec) {
    std::size_t seed = boost::hash_value(spec.node_id);
    boost::hash_combine(seed, spec.fd);
    return seed;
}
