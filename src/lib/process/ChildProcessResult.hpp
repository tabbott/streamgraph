#pragma once

#include <cstring>

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

struct ChildProcessResult {
    ChildProcessResult()
        : pid()
        , status()
        , rsrc()
    {}

    pid_t pid;
    int status;
    rusage rsrc;

    friend bool operator==(ChildProcessResult const& x, ChildProcessResult const& y) {
        return x.pid == y.pid
            && x.status == y.status
            && memcmp(&x.rsrc, &y.rsrc, sizeof(x.rsrc)) == 0;
    }
};
