#pragma once

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

struct ProcessResult {
    ProcessResult()
        : status()
        , rsrc()
    {}

    int status;
    rusage rsrc;
};
