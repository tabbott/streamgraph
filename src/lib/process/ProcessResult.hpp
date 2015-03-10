#pragma once

#include <cstring>

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace boost { namespace serialization {

    template<typename Archive>
    void serialize(Archive& arch, rusage& rsrc, unsigned version) {
        arch &
            rsrc.ru_maxrss &
            rsrc.ru_ixrss &
            rsrc.ru_idrss &
            rsrc.ru_isrss &
            rsrc.ru_minflt &
            rsrc.ru_majflt &
            rsrc.ru_nswap &
            rsrc.ru_inblock &
            rsrc.ru_oublock &
            rsrc.ru_msgsnd &
            rsrc.ru_msgrcv &
            rsrc.ru_nsignals &
            rsrc.ru_nvcsw &
            rsrc.ru_nivcsw
            ;
    }

}}

struct ProcessResult {
    ProcessResult()
        : pid()
        , status()
        , rsrc()
    {}

    pid_t pid;
    int status;
    rusage rsrc;

    template<typename Archive>
    void serialize(Archive& arch, unsigned version) {
        arch & pid & status & rsrc;
    }

    friend bool operator==(ProcessResult const& x, ProcessResult const& y) {
        return x.pid == y.pid
            && x.status == y.status
            && memcmp(&x.rsrc, &y.rsrc, sizeof(x.rsrc)) == 0;
    }
};
