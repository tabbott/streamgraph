#include "StatusXml.hpp"
#include "utility/TimeUtil.hpp"

#include <rapidxml_print.hpp>

#include <boost/format.hpp>

#include <fstream>
#include <stdexcept>

using boost::format;
using namespace rapidxml;

StatusXml::StatusXml(std::vector<Process::Ptr> const& processes)
    : root_(doc_.allocate_node(node_element, "streamgraph_status"))
{
    doc_.append_node(root_);
    for (auto iter = processes.begin(); iter != processes.end(); ++iter) {
        add_process(*iter);
    }
}

void StatusXml::write(std::string const& path) const {
    std::ofstream out(path.c_str());
    if (!out) {
        throw std::runtime_error(str(format(
            "Failed to open status xml file for writing: %1%"
            ) % path));
    }

    out << doc_;
}

void StatusXml::add_process(Process::Ptr const& proc) {
    auto proc_node = doc_.allocate_node(node_element, "process");
    proc_node->append_attribute(create_attr("name", proc->name()));
    proc_node->append_node(create_value_node("status", proc->raw_status()));
    proc_node->append_node(resource_node(proc->resource_usage()));
    root_->append_node(proc_node);
}

rapidxml::xml_node<>* StatusXml::resource_node(rusage const& rsrc) {
    auto node = doc_.allocate_node(node_element, "resource_usage");
    node->append_node(create_value_node("user_time_sec", tv2sec(rsrc.ru_utime)));
    node->append_node(create_value_node("system_time_sec", tv2sec(rsrc.ru_stime)));
    node->append_node(create_value_node("max_rss_kb", rsrc.ru_maxrss));
    node->append_node(create_value_node("minor_page_faults", rsrc.ru_minflt));
    node->append_node(create_value_node("major_page_faults", rsrc.ru_majflt));
    node->append_node(create_value_node("fs_blocks_in", rsrc.ru_inblock));
    node->append_node(create_value_node("fs_blocks_out", rsrc.ru_oublock));
    node->append_node(create_value_node("voluntary_context_switches", rsrc.ru_nvcsw));
    node->append_node(create_value_node("involuntary_context_switches", rsrc.ru_nivcsw));
    return node;
}
