#include "process/ChildProcess.hpp"
#include "utility/Syscall.hpp"

#include <boost/format.hpp>
#include <glog/logging.h>

#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include <sys/wait.h>

using boost::format;

ChildProcess::ChildProcess(std::string const& name, ChildFunction function)
    : name_(name)
    , function_(function)
    , pid_(0)
    , state_(eNEW)
{}

pid_t ChildProcess::start() {
    if (state_ != eNEW) {
        // FIXME: log
        return pid();
    }

    state_ = eRUNNING;
    pid_ = fork();

    if (pid_ == -1) {
        throw std::runtime_error(str(format(
            "Fork failed: %1%") % strerror(errno)));
    }

    if (!pid_) {
        LOG(INFO) << "Child process: " << getpid() << ": " << name();
        LOG(INFO) << "Applying fd mapping";
        fd_map_.apply();

        function_();

        LOG(FATAL) << "Failed to execute work function";
    }

    // parent
    LOG(INFO) << "Spawned new child: " << pid_ << ": " << name();

    return pid_;
}

void ChildProcess::set_result(ChildProcessResult const& result) {
    if (state() != eRUNNING) {
        throw std::runtime_error(str(format(
            "Attempted to set result for process that isn't running (pid %1%)"
            ) % pid()));
    }

    if (result.pid != pid_) {
        throw std::runtime_error(str(format(
            "Attempted to set result for process %1% with values for unlrelated pid %2%"
            ) % pid() % result.pid));
    }

    state_ = eCOMPLETE;
    result_ = result;
}

bool ChildProcess::finish() {
    if (state() != eRUNNING) {
        throw std::runtime_error(str(format(
            "Attempted to wait for non running process %1%"
            ) % pid_));
    }

    pid_t caught;
    SYSCALL_RETRY_VAR(caught, wait4(pid_, &result_.status, 0, &result_.rsrc));

    LOG(INFO) << "Reaped process " << caught << " " << name()
        << ", status: " << result_.status << "\n";

    if (caught != pid_)
        return false;

    state_ = eCOMPLETE;

    return succeeded();
}

int ChildProcess::exit_status() const {
    return WEXITSTATUS(result_.status);
}

int ChildProcess::exit_signal() const {
    return WTERMSIG(result_.status);
}
