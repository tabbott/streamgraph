#include "FdTeeCommand.hpp"

#include "utility/FdTee.hpp"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

void FdTeeCommand::configureOptions() {
    opts_.add_options()
        ("input-fd,i",
            po::value<>(&in_fd_)->required(),
            "input file descriptor number (first positional arg)")

        ("output-fd,o",
            po::value<>(&out_fds_)->required(),
            "output file descriptor numbers (pos args 2+, can be specified many times)")
        ;

    pos_opts_.add("input-fd", 1);
    pos_opts_.add("output-fd", -1);
}

void FdTeeCommand::exec() {
    FdTee fdtee(in_fd_, out_fds_);
    fdtee();
}
