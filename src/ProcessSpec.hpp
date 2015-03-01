#pragma once

#include "CStringArrayAdaptor.hpp"
#include "FdMapping.hpp"

#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <iostream>
#include <cerrno>

#include <unistd.h>

class ProcessSpec {
public:
    typedef boost::shared_ptr<ProcessSpec> Ptr;

    ProcessSpec(std::vector<std::string> argv, FdMapping const& fd_map)
        : fd_map_(fd_map)
    {
        argv_.swap(argv);
    }

    void execute() {
        CStringArrayAdaptor argv(argv_);

        fd_map_.apply();
        execvp(argv.arr()[0], argv.arr().data());

        // we should only get here if exec fails.
        // we don't know what fds are going where at this point. it would be
        // bad to try to make a sound.
        std::exit(1);
    }

private:
    std::vector<std::string> argv_;
    FdMapping const& fd_map_;
};
