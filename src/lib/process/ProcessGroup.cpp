#include "process/ProcessGroup.hpp"

#include <boost/format.hpp>

#include <stdexcept>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

ProcessGroup::ProcessGroup()
    : started_(false)
{}

void ProcessGroup::add(Process::Ptr const& proc) {
    /*
     *if (proc->state() != eNEW) {
     *    // XXX add process name or args or something here
     *    // maybe state name too
     *    throw std::runtime_error(
     *        "Attempted to add process in a state other than new"
     *        );
     *}
     */

    procs_.push_back(proc);
}

void ProcessGroup::start() {
    if (started_)
        return;

    std::size_t n = procs_.size();
    for (std::size_t i = 0; i < n; ++i) {
        procs_[i]->start();
    }
}

bool ProcessGroup::finish() {
    bool rv = true;
    std::size_t n = procs_.size();
    for (std::size_t i = 0; i < n; ++i) {
        Process& p = *procs_[i];
        if (p.state() == eRUNNING)
            p.finish();

        rv &= p.succeeded();
    }
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


