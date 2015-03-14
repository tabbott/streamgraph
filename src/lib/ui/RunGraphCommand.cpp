#include "RunGraphCommand.hpp"

#include "process/GraphXml.hpp"
#include "process/StatusXml.hpp"

#include <glog/logging.h>

#include <boost/program_options.hpp>

#include <iostream>
#include <sstream>

namespace po = boost::program_options;

void RunGraphCommand::configureOptions() {
    opts_.add_options()
        ("input-xml,x",
            po::value<>(&input_xml_path_)->required(),
            "input .xml file containing stream graph definition (also first positional arg)")

        ("output-xml,o",
            po::value<>(&output_xml_path_)->required(),
            "output .xml file containing process status information")
        ;

    pos_opts_.add("input-xml", 1);
}

void RunGraphCommand::exec() {
    FLAGS_logtostderr = true;
    ::google::InitGoogleLogging(executable_name().c_str());

    LOG(INFO) << "Executing: " << input_xml_path_
        << ", writing status to: " << output_xml_path_
        << "\n";

    GraphXml xml(input_xml_path_);
    ProcessGraph& pg = xml.graph();
    std::vector<std::string> fdtee_path{executable_name(), "fdtee"};
    pg.set_fdtee_cmd(fdtee_path);

    bool rv = pg.execute();
    std::cerr << "execute ok: " << std::boolalpha << rv << "\n";

    StatusXml status(pg.processes());
    status.write(output_xml_path_);

    if (!rv) {
        auto n = pg.size();
        std::stringstream ss;
        ss << "Some commands did not complete successfully:\n\n\tstatus\tname\tcmdline\n";
        for (ProcessGraph::NodeId i = 0; i < n; ++i) {
            auto const& p = pg.process(i);
            ss << "\t" << p->raw_status()
                << "\t" << p->name()
                << "\n";
        }

        throw std::runtime_error(ss.str());
    }
}
