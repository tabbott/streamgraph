#pragma once

#include <vector>
#include <string>

struct FdTee {
    FdTee(int src_fd, std::vector<int> dst_fds);

    void operator()() const;

    std::vector<std::string> const& args() const;

    int src_fd;
    std::vector<int> dst_fds;

private:
    std::vector<std::string> args_;
};
