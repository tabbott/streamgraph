#pragma once

#include <vector>

struct FdTee {
    FdTee(int src_fd, std::vector<int> dst_fds);

    void operator()() const;

    int src_fd;
    std::vector<int> dst_fds;
};
