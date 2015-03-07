#pragma once

#include "utility/FdMapping.hpp"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>
#include <string>

#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

enum ProcessState {
    eNEW,
    eRUNNING,
    eCOMPLETE
};

class Process : public boost::noncopyable {
public:
    typedef boost::shared_ptr<Process> Ptr;

    static Ptr create(std::vector<std::string> const& args);

    pid_t start();
    bool finish();

    pid_t pid() const               { return pid_; }
    ProcessState state() const      { return state_; }
    int raw_status() const          { return status_; }
    rusage resource_usage() const   { return rsrc_; }
    FdMapping& fd_map()             { return fd_map_; }

    bool succeeded() const {
        return state_ == eCOMPLETE && status_ == 0;
    }

    int exit_status() const;
    int exit_signal() const;

    std::string args_string() const;

private:
    Process(std::vector<std::string> const& args);

private:
    pid_t pid_;
    ProcessState state_;
    int status_;
    rusage rsrc_;
    std::vector<std::string> args_;
    FdMapping fd_map_;
};
