#pragma once

#include "process/Process.hpp"

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

private:
    bool started_;
    std::vector<Process::Ptr> procs_;
};
