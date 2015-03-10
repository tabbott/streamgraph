#include "process/ProcessGroup.hpp"
#include "utility/FdUtil.hpp"

#include <boost/format.hpp>
#include <boost/unordered_map.hpp>

#include <glog/logging.h>

#include <stdexcept>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

ProcessGroup::ProcessGroup()
    : started_(false)
    , runner_pid_(-1)
{}

void ProcessGroup::add(Process::Ptr const& proc) {
    procs_.push_back(proc);
}

bool ProcessGroup::runner_execute(int output_pipe) {
    std::size_t n = procs_.size();

    typedef boost::unordered_map<pid_t, std::size_t> PidMap;
    PidMap pidmap;

    results_.resize(n);

    LOG(INFO) << "Process group starting " << n << " processes.";
    for (std::size_t i = 0; i < n; ++i) {
        procs_[i]->start();
        pidmap[procs_[i]->pid()] = i;
    }
    LOG(INFO) << "Process group started " << n << " processes.";
    LOG(INFO) << "Closing all nonstd file descriptors";

    FdMapping fdmap;
    fdmap.add_existing_fd(0, 0);
    fdmap.add_existing_fd(1, 1);
    fdmap.add_existing_fd(2, 2);
    fdmap.apply();

    ProcessResult result;
    std::size_t n_caught = 0;
    bool rv = true;
    while (n_caught < n) {
        result.pid = wait4(-1, &result.status, 0, &result.rsrc);
        if (result.pid == -1 && errno == EINTR)
            continue;

        PCHECK(result.pid >= 0) << "while waiting for child processes: wait4 failed";

        PidMap::iterator found = pidmap.find(result.pid);
        if (found == pidmap.end()) {
            LOG(WARNING) << "caught unexpected child pid " << result.pid << ", ignoring.";
            continue;
        }

        ++n_caught;

        std::size_t idx = found->second;
        LOG(INFO) << "Reaped child " << result.pid << " (" << procs_[idx]->args_string()
            << "), status = " << result.status;

        results_[idx] = result;
        rv &= result.status == 0;
    }
    return rv;
}

void ProcessGroup::start() {
    runner_pid_ = fork();
    PCHECK(runner_pid_ != -1) << "fork failed";
    if (runner_pid_ == 0) {
        int rv = runner_execute(0);
        std::exit(!rv);
    }

#if 0
    if (started_)
        return;

    std::size_t n = procs_.size();
    LOG(INFO) << "Process group starting " << n << " processes.";
    for (std::size_t i = 0; i < n; ++i) {
        procs_[i]->start();
    }
    LOG(INFO) << "Process group started " << n << " processes.";
#endif
}

bool ProcessGroup::finish() {
    if (runner_pid_ == -1)
        throw std::runtime_error("finish called on process group that is not running");

    int status;
    pid_t caught;
    SYSCALL_RETRY_VAR(caught, waitpid(runner_pid_, &status, 0));
    PCHECK(caught != -1) << "waiting for process group failed.";

    runner_pid_ = -1;

    return status == 0;
#if 0
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
#endif
}

void ProcessGroup::signal_all(int signum) {
    std::size_t n = procs_.size();
    for (std::size_t i = 0; i < n; ++i) {
        Process& p = *procs_[i];
        if (p.state() == eRUNNING)
            kill(p.pid(), signum);
    }
}


