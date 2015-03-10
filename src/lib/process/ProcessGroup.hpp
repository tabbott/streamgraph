#pragma once

#include "process/Process.hpp"
#include "process/ProcessResult.hpp"

#include <boost/unordered_map.hpp>

#include <vector>

class ProcessGroup {
public:
    ProcessGroup();

    void add(Process::Ptr const& proc);

    void start();
    bool finish();

    void signal_all(int signum);

    std::vector<Process::Ptr> const& processes() const {
        return procs_;
    }

    std::vector<ProcessResult> const& results() const {
        return results_;
    }

private:
    typedef boost::unordered_map<pid_t, std::size_t> PidMap;

    bool started_;
    std::vector<Process::Ptr> procs_;
    std::vector<ProcessResult> results_;
    PidMap pidmap_;
};
