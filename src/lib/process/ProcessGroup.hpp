#pragma once

#include "process/ChildProcess.hpp"
#include "process/ChildProcessResult.hpp"

#include <boost/unordered_map.hpp>

#include <vector>

class ProcessGroup {
public:
    ProcessGroup();

    void add(ChildProcess::Ptr const& proc);

    void start();
    bool finish();

    void signal_all(int signum);

    std::vector<ChildProcess::Ptr> const& processes() const {
        return procs_;
    }

    std::vector<ChildProcessResult> const& results() const {
        return results_;
    }

private:
    typedef boost::unordered_map<pid_t, std::size_t> PidMap;

    bool started_;
    std::vector<ChildProcess::Ptr> procs_;
    std::vector<ChildProcessResult> results_;
    PidMap pidmap_;
};
