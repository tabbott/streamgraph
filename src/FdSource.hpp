#pragma once

#include <cerrno>
#include <string>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

class FdSourceBase {
public:
    virtual ~FdSourceBase() {}

    virtual int get_fd() = 0;
};


class FdSource : public FdSourceBase {
public:
    FdSource(int src_fd);
    int get_fd();

private:
    int fd_;
};


class FileFdSource : public FdSourceBase {
public:
    FileFdSource(std::string const& path, int flags, mode_t mode);
    int get_fd();

private:
    std::string path_;
    int flags_;
    mode_t mode_;
};
