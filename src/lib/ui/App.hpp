#pragma once

#include "ui/CommandBase.hpp"

#include <ostream>
#include <map>

class App {
public:
    App();

    void exec(int argc, char** argv);

private:
    void register_command(CommandBase::ptr const& cmd);
    void describe_commands(std::ostream& os);

private:
    std::map<std::string, CommandBase::ptr> cmds_;
};
