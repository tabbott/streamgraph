#include "process/Process.hpp"
#include "utility/CStringArrayAdaptor.hpp"
#include "utility/FdUtil.hpp"

#include <boost/format.hpp>
#include <glog/logging.h>

#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include <sys/wait.h>

using boost::format;

Process::Ptr Process::create(std::vector<std::string> const& args) {
    return boost::shared_ptr<Process>(new Process(args));
}

Process::Process(std::vector<std::string> const& args)
    : pid_(0)
    , state_(eNEW)
    , status_(0)
    , rsrc_()
    , args_(args)
{}

pid_t Process::start() {
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
        LOG(INFO) << "Child process: " << getpid() << ": " << args_string();
        LOG(INFO) << "Applying fd mapping";
        fd_map_.apply();
        CStringArrayAdaptor argv(args_);
        execvp(argv.arr()[0], argv.arr().data());
        LOG(FATAL) << "Failed to execute " << argv.arr()[0] << ": " << strerror(errno);
    }

    // parent
    LOG(INFO) << "Spawned new child: " << pid_ << ": " << args_string();

    return pid_;
}

bool Process::finish() {
    if (state() != eRUNNING) {
        throw std::runtime_error(str(format(
            "Attempted to wait for non running process %1%"
            ) % pid_));
    }

    pid_t caught;
    // FIXME: mask stop/cont
    while ((caught = wait4(pid_, &status_, 0, &rsrc_)) && errno == EINTR)
        ;

    LOG(INFO) << "Reaped process " << caught << " (" << args_string()
        << "), status: " << status_ << "\n";

    if (caught != pid_)
        return false;

    state_ = eCOMPLETE;

    return succeeded();
}

std::string Process::args_string() const {
    std::stringstream ss;
    for (std::size_t i = 0; i < args_.size(); ++i) {
        if (i > 0)
            ss << " ";
        ss << args_[i];
    }
    return ss.str();
}

int Process::exit_status() const {
    return WEXITSTATUS(status_);
}

int Process::exit_signal() const {
    return WTERMSIG(status_);
}
