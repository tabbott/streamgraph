#include "ProcessGraph.hpp"

#include "process/Process.hpp"
#include "process/ProcessGroup.hpp"

#include <glog/logging.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <stdexcept>

#include <string>
#include <vector>

using boost::format;

ProcessGraph::ProcessGraph(std::vector<std::string> fdtee_cmd)
    : fdtee_cmd_(fdtee_cmd)
{}

ProcessGraph::NodeId ProcessGraph::add(Process::Ptr const& proc) {
    pgroup_.add(proc);
    nodes_.push_back(proc);
    return nodes_.size() - 1;
}

ProcessGraph::NodeId ProcessGraph::add(std::string const& name, std::vector<std::string> const& args) {
    return add(Process::create(name, args));
}

ProcessGraph::NodeId ProcessGraph::size() const {
    return nodes_.size();
}

Process::Ptr& ProcessGraph::process(NodeId id) {
    validate_node(id);
    return nodes_[id];
}

Process::Ptr const& ProcessGraph::process(NodeId id) const {
    validate_node(id);
    return nodes_[id];
}

void ProcessGraph::validate_node(NodeId id) const {
    if (id >= nodes_.size()) {
        LOG(ERROR) << "Request for invalid node id " << id;
        throw std::runtime_error(str(format(
            "Request for invalid node id %1%"
            ) % id));
    }
}

void ProcessGraph::connect(NodeId src, int src_fd, NodeId dst, int dst_fd) {
    validate_node(src);
    validate_node(dst);
    FdSpec a = {src, src_fd};
    FdSpec b = {dst, dst_fd};
    std::pair<InPipeMap::iterator, bool> inserted = in_pipes_.insert(std::make_pair(b, a));
    if (!inserted.second) {
        if (inserted.first->second != a) {
            LOG(ERROR) << "Attempted to connect multiple streams to process "
                << dst << " fd " << dst_fd << " (" << process(dst)->args_string() << ")";
            throw std::runtime_error(str(format(
                "Attempted to connect multiple streams to target (proc %1% fd %2%)"
                ) % dst % dst_fd));
        }
        LOG(WARNING) << "Redundant connection of (process "
            << src << " fd " << src_fd << ") -> (process "
            << dst << " fd " << dst_fd << ")";
    }
    pipes_[a].insert(b);
}

void ProcessGraph::connect_input_file(std::string const& src, NodeId dst, int dst_fd) {
    validate_node(dst);
    process(dst)->fd_map().add_file(dst_fd, src, O_RDONLY, 0644);
}

void ProcessGraph::connect_output_file(NodeId src, int src_fd, std::string const& dst,
    int mode /*= 0644*/, bool append /*= false*/)
{
    validate_node(src);
    int flags = O_WRONLY | O_CREAT;
    if (append) {
        flags |= O_APPEND;
    }
    else {
        flags |= O_TRUNC;
    }
    process(src)->fd_map().add_file(src_fd, dst, flags, mode);
}


void ProcessGraph::create_pipe(int rwpipe[2]) {
    if (pipe(rwpipe)) {
        throw std::runtime_error("failed to open pipe");
    }

    LOG(INFO) << "Created new pipe. (r, w) fds: ("
        << rwpipe[0] << ", " << rwpipe[1] << ")";
}

bool ProcessGraph::execute() {
    std::vector<FdSpec> broadcasters;

    for (OutPipeMap::const_iterator iter = pipes_.begin(); iter != pipes_.end(); ++iter) {
        FdSpecSet const& destinations = iter->second;
        if (destinations.size() > 1) {
            broadcasters.push_back(iter->first);
        }
    }

    for (std::size_t i = 0; i < broadcasters.size(); ++i) {
        FdSpec const& src = broadcasters[i];
        OutPipeMap::iterator iter = pipes_.find(src);
        NodeId tee_id = add(make_fdtee_cmd(3, iter->second.size()));
        FdSpecSet dst_copy = iter->second;
        pipes_.erase(iter);
        connect(src.node_id, src.fd, tee_id, 3);
        int tee_fd = 4;
        for (FdSpecSet::const_iterator dst_iter = dst_copy.begin(); dst_iter != dst_copy.end(); ++dst_iter) {
            in_pipes_.erase(*dst_iter);
            connect(tee_id, tee_fd++, dst_iter->node_id, dst_iter->fd);
        }
    }

    for (OutPipeMap::const_iterator siter = pipes_.begin(); siter != pipes_.end(); ++siter) {
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

Process::Ptr ProcessGraph::make_fdtee_cmd(int read_fd, std::size_t n_dst) const {
    std::vector<std::string> args = fdtee_cmd_;
    args.push_back(boost::lexical_cast<std::string>(read_fd));
    for (std::size_t i = 1; i <= n_dst; ++i) {
        args.push_back(boost::lexical_cast<std::string>(read_fd + i));
    }
    return Process::create(str(format("__fdtee_%1%") % nodes_.size()), args);
}
