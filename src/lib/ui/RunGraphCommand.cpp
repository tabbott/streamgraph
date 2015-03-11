#include "RunGraphCommand.hpp"

#include "process/GraphXml.hpp"

#include <glog/logging.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <iostream>

namespace bfs = boost::filesystem;
namespace po = boost::program_options;

void RunGraphCommand::configureOptions() {
    opts_.add_options()
        ("input-xml,x",
            po::value<>(&xml_path_)->required(),
            "input .xml file containing stream graph definition (also first positional arg)")

        ("log-directory,l",
            po::value<>(&log_dir_)->required(),
            "output directory for log files")
        ;

    pos_opts_.add("input-xml", 1);
}

void RunGraphCommand::exec() {
    std::cerr << "Executing: " << xml_path_ << ", logging to: " << log_dir_ << "\n";

    if (!bfs::is_directory(log_dir_)) {
        std::cerr << "Creating log directory " << log_dir_ << "\n";
        bfs::create_directories(log_dir_);
    }

    FLAGS_log_dir = log_dir_;
    ::google::InitGoogleLogging(executable_name().c_str());

    GraphXml xml(xml_path_);
    ProcessGraph& pg = xml.graph();
    std::vector<std::string> fdtee_path{executable_name(), "fdtee"};
    pg.set_fdtee_cmd(fdtee_path);

    bool rv = pg.execute();
    std::cerr << "execute ok: " << std::boolalpha << rv << "\n";
}
