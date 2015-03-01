#include "FdSource.hpp"

FdSource::FdSource(int src_fd)
    : fd_(src_fd)
{}

int FdSource::get_fd() {
    return fd_;
}


FileFdSource::FileFdSource(std::string const& path, int flags, mode_t mode)
    : path_(path)
    , flags_(flags)
    , mode_(mode)
{}

int FileFdSource::get_fd() {
    int fd = -1;
    while ((fd = open(path_.c_str(), flags_, mode_)) == -1 && errno == EINTR)
        ;
    return fd;
}
