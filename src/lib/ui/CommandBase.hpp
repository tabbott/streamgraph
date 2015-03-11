#pragma once

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include <stdexcept>
#include <string>
#include <vector>

class CmdlineHelpException : public std::runtime_error {
public:
    CmdlineHelpException (std::string const& msg)
        : std::runtime_error(msg)
    {
    }
};

class OptionsReparsedException : public std::runtime_error {
public:
    OptionsReparsedException(std::string const& msg)
        : std::runtime_error(msg)
    {
    }
};


class CommandBase {
public:
    typedef boost::shared_ptr<CommandBase> ptr;

    CommandBase();
    virtual ~CommandBase() {}

    virtual void exec() = 0;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
    virtual bool hidden() const {
        return false;
    }

    void parse_args(int argc, char** argv, int level);

    std::string const& executable_name() const {
        return executable_name_;
    }

protected:
    virtual void configureOptions() {}
    virtual void finalizeOptions() {}

private:
    void parse_args(std::vector<std::string> const& args);
    void checkHelp() const;

protected:
    std::string executable_name_;
    bool options_parsed_;
    boost::program_options::options_description opts_;
    boost::program_options::positional_options_description pos_opts_;
    boost::shared_ptr<boost::program_options::parsed_options> parsed_args_;
    boost::program_options::variables_map var_map_;
};
