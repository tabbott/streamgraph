#include "App.hpp"

#include "FdTeeCommand.hpp"
#include "RunGraphCommand.hpp"

#include <boost/format.hpp>

#include <stdexcept>
#include <cassert>

using boost::format;

App::App() {
    register_command(CommandBase::ptr(new FdTeeCommand));
    register_command(CommandBase::ptr(new RunGraphCommand));
}

void App::register_command(CommandBase::ptr const& cmd) {
    assert(cmd);
    auto inserted = cmds_.insert(std::make_pair(cmd->name(), cmd));
    if (!inserted.second) {
        throw std::runtime_error(str(format("Command name collision: %1%") % cmd->name()));
    }
}

void App::exec(int argc, char** argv) {
    std::stringstream cmd_help;
    cmd_help << "Valid subcommands:\n\n";
    describe_commands(cmd_help);

    if (argc < 2) {
        throw std::runtime_error(str(format(
            "No subcommand specified. %1%"
            ) % cmd_help.str()));
    }

    std::string cmdstr = argv[1];
    if (cmdstr == "-h" || cmdstr == "--help")
        throw CmdlineHelpException(cmd_help.str());

    auto found = cmds_.find(cmdstr);
    if (found == cmds_.end())
        throw std::runtime_error(str(format("Invalid subcommand '%1%'. %2%"
        ) % cmdstr % cmd_help.str()));

    auto& cmd = found->second;
    cmd->parse_args(argc, argv, 2);
    cmd->exec();
}

void App::describe_commands(std::ostream& os) {
    for (auto iter = cmds_.begin(); iter != cmds_.end(); ++iter) {
        if (iter->second->hidden())
            continue;
        os << "\t" << iter->second->name() << " - "
            << iter->second->description() << "\n";
    }
}
