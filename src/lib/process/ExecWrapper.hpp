#pragma once

#include <vector>
#include <string>

class ExecWrapper {
public:
    ExecWrapper(std::vector<std::string> args);

    void operator()();

private:
    std::vector<std::string> args_;
};
