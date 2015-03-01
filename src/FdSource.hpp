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
    FdSource(int src_fd)
        : fd_(src_fd)
    {}

    int get_fd() {
        return fd_;
    }

private:
    int fd_;
};



class FileFdSource : public FdSourceBase {
public:
    FileFdSource(std::string const& path, int flags, mode_t mode)
        : path_(path)
        , flags_(flags)
        , mode_(mode)
    {}

    int get_fd() {
        int fd = -1;
        while ((fd = open(path_.c_str(), flags_, mode_)) == -1 && errno == EINTR)
            ;
        return fd;
    }

private:
    std::string path_;
    int flags_;
    mode_t mode_;
};
