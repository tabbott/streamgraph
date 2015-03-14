#pragma once

#include <boost/iostreams/device/mapped_file.hpp>
#include <fstream>
#include <stdexcept>
#include <string>

namespace io {

    inline
    std::string read_file(std::string const& path) {
        boost::iostreams::mapped_file_source src(path);
        return std::string(src.data(), src.size());
    }

    inline
    ssize_t safe_read(int fd, void* buf, size_t count) {
        ssize_t rv = 0;
        while ((rv = read(fd, buf, count)) == -1 && errno == EINTR)
            ;
        return rv;
    }

    inline
    ssize_t safe_write(int fd, void const* buf, size_t count) {
        ssize_t rv = 0;
        while ((rv = write(fd, buf, count)) == -1 && errno == EINTR)
            ;
        return rv;
    }

    inline
    ssize_t safe_write_all(int fd, char const* buf, size_t count) {
        ssize_t rv = 0;
        ssize_t remain = count;
        char const* p = buf;
        while (remain > 0 && (rv = safe_write(fd, p, remain)) > 0) {
            remain -= rv;
            p += rv;
        }
        return rv < 0 ? rv : (count - remain);
    }
}
