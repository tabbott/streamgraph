#pragma once

#include "ui/CommandBase.hpp"

class FdTeeCommand : public CommandBase {
public:
    std::string name() const { return "fdtee"; }

    std::string description() const {
        return "tee for file descriptors (for internal use)";
    }

    void configureOptions();
    void exec();

private:
    int in_fd_;
    std::vector<int> out_fds_;
};
