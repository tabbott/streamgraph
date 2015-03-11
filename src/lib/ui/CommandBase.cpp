#include "ui/CommandBase.hpp"

#include <boost/format.hpp>
#include <boost/make_shared.hpp>

#include <sstream>
#include <stdexcept>

#include <iostream>

namespace po = boost::program_options;
using boost::format;
using namespace std;

CommandBase::CommandBase()
    : options_parsed_(false)
{
}

void CommandBase::parse_args(std::vector<std::string> const& args) {
    if (options_parsed_) {
        throw OptionsReparsedException(str(format(
            "Attempted to parse command line options multiple times in "
            "command %1%!")
            % name()));
    }

    opts_.add_options()
        ("help,h", "this message")
        ;

    configureOptions();

    try {

        auto parsedArgs = po::command_line_parser(args)
                .options(opts_)
                .positional(pos_opts_).run();

        po::store(parsedArgs, var_map_);

        parsed_args_ = boost::make_shared<po::parsed_options>(parsedArgs);

        po::notify(var_map_);

    } catch (...) {
        // program options will throw if required options are not passed
        // before we have a chance to check if the user has asked for
        // --help. If they have, let's give it to them, otherwise, rethrow.
        checkHelp();
        throw;
    }

    checkHelp();
    finalizeOptions();
}

void CommandBase::checkHelp() const {
    if (var_map_.count("help")) {
        stringstream ss;
        ss << "Command: " << name() << "\n\n"
            << "Description:\n" << description() << "\n\n"
            << "Options:\n" << opts_;
        throw CmdlineHelpException(ss.str());
    }
}

void CommandBase::parse_args(int argc, char** argv, int level) {
    assert(argc > 0);
    executable_name_ = argv[0];
    std::vector<std::string> args(argv + level, argv + argc);
    parse_args(args);
}
