#pragma once

#include "process/Process.hpp"
#include "process/ProcessResult.hpp"

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
    bool runner_execute(int output_pipe);

private:
    bool started_;
    pid_t runner_pid_;
    std::vector<Process::Ptr> procs_;
    std::vector<ProcessResult> results_;
};
