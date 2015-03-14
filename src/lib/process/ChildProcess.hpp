#pragma once

#include "process/ChildFunction.hpp"
#include "process/ChildProcessResult.hpp"
#include "utility/FdMapping.hpp"

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <functional>
#include <string>
#include <vector>

#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>


enum ChildProcessState {
    eNEW,
    eRUNNING,
    eCOMPLETE
};

class ChildProcess : public boost::noncopyable {
public:
    typedef boost::shared_ptr<ChildProcess> Ptr;

    template<typename T>
    static Ptr create(std::string const& name, T const& function) {
        return Ptr(new ChildProcess(name, ChildFunction(function)));
    }

    pid_t start();
    bool finish();

    pid_t pid() const               { return pid_; }
    ChildProcessState state() const   { return state_; }
    int raw_status() const          { return result_.status; }
    rusage resource_usage() const   { return result_.rsrc; }
    FdMapping& fd_map()             { return fd_map_; }
    std::string const& name() const { return name_; }

    void set_result(ChildProcessResult const& status);
    ChildProcessResult const& result() const {
        return result_;
    }

    bool succeeded() const {
        return state_ == eCOMPLETE && result_.status == 0;
    }

    int exit_status() const;
    int exit_signal() const;

private:
    ChildProcess(std::string const& name, ChildFunction function);

private:
    std::string name_;
    ChildFunction function_;

    pid_t pid_;
    ChildProcessState state_;
    ChildProcessResult result_;
    FdMapping fd_map_;
};
