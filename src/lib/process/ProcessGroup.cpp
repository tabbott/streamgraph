#include "process/ProcessGroup.hpp"

#include <boost/format.hpp>

#include <glog/logging.h>

#include <stdexcept>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

ProcessGroup::ProcessGroup()
    : started_(false)
{}

void ProcessGroup::add(Process::Ptr const& proc) {
    procs_.push_back(proc);
}

void ProcessGroup::start() {
    if (started_)
        return;

    std::size_t n = procs_.size();
    LOG(INFO) << "Process group starting " << n << " processes.";
    for (std::size_t i = 0; i < n; ++i) {
        procs_[i]->start();
    }
    LOG(INFO) << "Process group started " << n << " processes.";
}

bool ProcessGroup::finish() {
    bool rv = true;
    std::size_t n = procs_.size();
    LOG(INFO) << "Process group waiting for " << n << " processes.";
    for (std::size_t i = 0; i < n; ++i) {
        Process& p = *procs_[i];
        if (p.state() == eRUNNING)
            p.finish();

        rv &= p.succeeded();
    }
    LOG(INFO) << "Process group waited for " << n << " processes. "
        << "All ok? " << rv << ".";
    return rv;
}

void ProcessGroup::signal_all(int signum) {
    std::size_t n = procs_.size();
    for (std::size_t i = 0; i < n; ++i) {
        Process& p = *procs_[i];
        if (p.state() == eRUNNING)
            kill(p.pid(), signum);
    }
}


