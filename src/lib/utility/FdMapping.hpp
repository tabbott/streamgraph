#pragma once

#include <boost/noncopyable.hpp>

#include <string>

#include <sys/types.h>
#include <fcntl.h>

class FdMapping : public boost::noncopyable {
public:
    FdMapping();
    ~FdMapping();

    void add_existing_fd(int target_fd, int source_fd);
    void add_file(int target_fd, std::string const& path, int flags, mode_t mode);

    void apply() const;

private:
    class Impl;
    Impl* impl_;
};
