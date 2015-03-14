#pragma once

#include <cstdlib>
#include <functional>

// This class is intended to be used in a child process (i.e., after fork)
// The provided work function is expected to call exit(). If it fails to do
// so, exit(1) will be called.
//
// The reason for providing this class is to prevent users from forgetting
// to call exit and having a child process escape back into code that is
// intended to be executed by the parent.
class ChildFunction {
public:
    typedef std::function<void()> WorkFunction;

    ChildFunction(WorkFunction function)
        : function_(function)
    {}

    void operator()() {
        function_();
        std::exit(1);
    }

private:
    WorkFunction function_;
};
