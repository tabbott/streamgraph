#pragma once

#include <cerrno>

#define SYSCALL_RETRY(call) \
    while ( ((call) == -1) && errno == EINTR);

#define SYSCALL_RETRY_VAR(var, call) \
    while ( (((var) = (call)) == -1) && errno == EINTR);


