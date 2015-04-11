#include "FdTee.hpp"
#include "utility/io.hpp"

#include <boost/lexical_cast.hpp>

#include <cerrno>
#include <utility>

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

namespace {
    // based on size of linux pipe buffer (2^16 since linux 2.6.11)
    const std::size_t BUFSZ = 1 << 16;

    int fdtee(int src_fd, std::vector<int> const& dst_fds) {
        std::size_t n_dst_fds = dst_fds.size();

        ssize_t bytes_in;
        char buf[BUFSZ];
        while ((bytes_in = io::safe_read(src_fd, buf, BUFSZ)) > 0) {

            for (std::size_t i = 0; i < n_dst_fds; ++i) {
                int bytes_out;
                if ((bytes_out = io::safe_write_all(dst_fds[i], buf, bytes_in)) != bytes_in) {
                    return 10 + i;
                }
            }
        }

        if (bytes_in == -1)
            return 1;

        return 0;
    }
}

FdTee::FdTee(int src_fd, std::vector<int> dst_fds_)
    : src_fd(src_fd)
    , dst_fds(std::move(dst_fds_))
{
    args_.reserve(dst_fds.size() + 2);
    args_.push_back("$internal_fdtee$");
    args_.push_back(boost::lexical_cast<std::string>(src_fd));
    for (auto i = dst_fds.begin(); i != dst_fds.end(); ++i) {
        args_.push_back(boost::lexical_cast<std::string>(*i));
    }
}

void FdTee::operator()() const {
    std::exit(fdtee(src_fd, dst_fds));
}

std::vector<std::string> const& FdTee::args() const {
    return args_;
}
