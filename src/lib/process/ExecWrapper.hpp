#pragma once

#include <vector>
#include <string>

class ExecWrapper {
public:
    ExecWrapper(std::vector<std::string> args);

    void operator()();
    std::vector<std::string> const& args() const {
        return args_;
    }

private:
    std::vector<std::string> args_;
};
