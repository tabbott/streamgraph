#pragma once

#include "ui/CommandBase.hpp"

class RunGraphCommand : public CommandBase {
public:
    std::string name() const { return "run"; }

    std::string description() const {
        return "run a stream graph from an xml description";
    }

    void configureOptions();
    void exec();

private:
    std::string log_dir_;
    std::string xml_path_;
};
