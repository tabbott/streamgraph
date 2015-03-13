#include "FdTee.hpp"

#include <cerrno>

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>


namespace {
    // based on size of linux pipe buffer (2^16 since linux 2.6.11)
    const std::size_t BUFSZ = 1 << 16;

    ssize_t safe_read(int fd, void* buf, size_t count) {
        ssize_t rv = 0;
        while ((rv = read(fd, buf, count)) == -1 && errno == EINTR)
            ;
        return rv;
    }

    ssize_t safe_write(int fd, void const* buf, size_t count) {
        ssize_t rv = 0;
        while ((rv = write(fd, buf, count)) == -1 && errno == EINTR)
            ;
        return rv;
    }

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

int fdtee(int src_fd, std::vector<int> const& dst_fds) {
    std::size_t n_dst_fds = dst_fds.size();

    ssize_t bytes_in;
    char buf[BUFSZ];
    while ((bytes_in = safe_read(src_fd, buf, BUFSZ)) > 0) {

        for (std::size_t i = 0; i < n_dst_fds; ++i) {
            int bytes_out;
            if ((bytes_out = safe_write_all(dst_fds[i], buf, bytes_in)) != bytes_in) {
                return 10 + i;
            }
        }
    }

    if (bytes_in == -1)
        return 1;

    return 0;
}
