#include "process/ProcessGroup.hpp"
#include "utility/FdUtil.hpp"

#include <boost/format.hpp>

#include <glog/logging.h>

#include <stdexcept>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

ProcessGroup::ProcessGroup()
    : started_(false)
{}

void ProcessGroup::add(Process::Ptr const& proc) {
    procs_.push_back(proc);
}

void ProcessGroup::start() {
    std::size_t n = procs_.size();

    results_.resize(n);

    LOG(INFO) << "Process group starting " << n << " processes.";
    for (std::size_t i = 0; i < n; ++i) {
        procs_[i]->start();
        pidmap_[procs_[i]->pid()] = i;
    }
    LOG(INFO) << "Process group started " << n << " processes.";
}

bool ProcessGroup::finish() {
    std::size_t n = procs_.size();

    ProcessResult result;
    std::size_t n_caught = 0;
    bool rv = true;
    while (n_caught < n) {
        result.pid = wait4(-1, &result.status, 0, &result.rsrc);
        if (result.pid == -1 && errno == EINTR)
            continue;

        PCHECK(result.pid >= 0) << "while waiting for child processes: wait4 failed";

        PidMap::iterator found = pidmap_.find(result.pid);
        if (found == pidmap_.end()) {
            LOG(WARNING) << "caught unexpected child pid " << result.pid << ", ignoring.";
            continue;
        }

        ++n_caught;

        std::size_t idx = found->second;
        LOG(INFO) << "Reaped child " << result.pid << " (" << procs_[idx]->args_string()
            << "), status = " << result.status;

        procs_[idx]->set_result(result);
        results_[idx] = result;
        rv &= result.status == 0;
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


