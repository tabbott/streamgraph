#include "process/ProcessGraph.hpp"
#include "process/ChildProcess.hpp"
#include "process/ExecWrapper.hpp"
#include "process/ProcessGroup.hpp"
#include "utility/FdTee.hpp"

#include <glog/logging.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

using boost::format;

ProcessGraph::NodeId ProcessGraph::add(ChildProcess::Ptr const& proc) {
    pgroup_.add(proc);
    nodes_.push_back(proc);
    return nodes_.size() - 1;
}

ProcessGraph::NodeId ProcessGraph::add(std::string const& name, std::vector<std::string> const& args) {
    ExecWrapper wrapper(args);
    return add(ChildProcess::create(name, wrapper));
}

ProcessGraph::NodeId ProcessGraph::size() const {
    return nodes_.size();
}

ChildProcess::Ptr& ProcessGraph::process(NodeId id) {
    validate_node(id);
    return nodes_[id];
}

ChildProcess::Ptr const& ProcessGraph::process(NodeId id) const {
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
    auto inserted = in_pipes_.insert(std::make_pair(b, a));
    if (!inserted.second) {
        if (inserted.first->second != a) {
            LOG(ERROR) << "Attempted to connect multiple streams to process "
                << dst << " fd " << dst_fd << " (" << process(dst)->name() << ")";
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

void ProcessGraph::expand_fanout() {
    std::vector<FdSpec> broadcasters;

    for (auto iter = pipes_.begin(); iter != pipes_.end(); ++iter) {
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
        for (auto dst_iter = dst_copy.begin(); dst_iter != dst_copy.end(); ++dst_iter) {
            in_pipes_.erase(*dst_iter);
            connect(tee_id, tee_fd++, dst_iter->node_id, dst_iter->fd);
        }
    }
}

bool ProcessGraph::execute() {
    expand_fanout();

    std::vector<int> pipe_fds;

    for (auto iter = pipes_.begin(); iter != pipes_.end(); ++iter) {
        int rwpipe[2];
        create_pipe(rwpipe);

        pipe_fds.push_back(rwpipe[0]);
        pipe_fds.push_back(rwpipe[1]);

        FdSpec const& src = iter->first;
        FdSpecSet const& destinations = iter->second;

        // this should be guaranteed by calling flatten
        assert(destinations.size() == 1);
        FdSpec const& dst = *destinations.begin();
        ChildProcess::Ptr& src_proc = process(src.node_id);
        ChildProcess::Ptr& dst_proc = process(dst.node_id);
        src_proc->fd_map().add_existing_fd(src.fd, rwpipe[1]);
        dst_proc->fd_map().add_existing_fd(dst.fd, rwpipe[0]);

        LOG(INFO) << "piping (proc " << src.node_id << ", fd "
            << src.fd << " -> (proc " << dst.node_id << ", fd "
            << dst.fd  << ")";
    }

    pgroup_.start();
    std::for_each(pipe_fds.begin(), pipe_fds.end(), &close);
    return pgroup_.finish();
}

ChildProcess::Ptr ProcessGraph::make_fdtee_cmd(int read_fd, std::size_t n_dst) const {
    std::vector<int> write_fds;
    for (std::size_t i = 1; i <= n_dst; ++i) {
        write_fds.push_back(read_fd + i);
    }
    FdTee fdtee(read_fd, write_fds);
    return ChildProcess::create(str(format("__fdtee_%1%") % nodes_.size()), fdtee);
}

ProcessGraph::NodeList ProcessGraph::processes() const {
    return nodes_;
}

// FIXME: we should probably look at using boost graph to do this and a few
// other things in this module
void ProcessGraph::write_basic_dot(std::string const& path) const {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error(str(format(
            "Failed to open output dot file %1%"
            ) % path));
    }

    out << "digraph G {\n";
    out << "\trankdir=LR;\n";

    std::size_t n = nodes_.size();
    for (std::size_t i = 0; i < n; ++i) {
        auto const& proc = *nodes_[i];
        out << "\t" << i << " [label=\"" << proc.name() << "\"];\n";
    }

    for (auto iter = pipes_.begin(); iter != pipes_.end(); ++iter) {
        FdSpec const& src = iter->first;
        FdSpecSet const& dsts = iter->second;

        for (auto tgt = dsts.begin(); tgt != dsts.end(); ++tgt) {
            out << "\t" << src.node_id << " -> " << tgt->node_id << ";\n";
        }
    }

    out << "}\n";
}
