#include <boost/lexical_cast.hpp>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

namespace {
    // based on size of linux pipe buffer (2^16 since linux 2.6.11)
    const size_t BUFSZ = 1 << 16;
}

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

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <in_fd> <out_fd1> ...\n";
        return 1;
    }

    int in_fd = atoi(argv[1]);
    std::vector<int> out_fds;
    std::size_t n_out_fds = argc - 2;
    out_fds.reserve(n_out_fds);
    for (int i = 2; i < argc; ++i) {
        out_fds.push_back(atoi(argv[i]));
    }

    ssize_t bytes_in;
    char buf[BUFSZ];
    while ((bytes_in = safe_read(in_fd, buf, BUFSZ)) > 0) {

        for (std::size_t i = 0; i < n_out_fds; ++i) {
            int bytes_out;
            if ((bytes_out = safe_write_all(out_fds[i], buf, bytes_in)) != bytes_in) {
                return 10 + i;
            }
        }
    }

    if (bytes_in == -1)
        return 1;

    return 0;
}
