#pragma once

#include <sys/time.h>

inline
double tv2sec(timeval const& tv) {
    return tv.tv_sec + tv.tv_usec * 1e-6;
}
