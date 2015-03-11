#include "GraphXml.hpp"
#include "utility/io.hpp"

#include <glog/logging.h>

#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_numeric.hpp>

#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <rapidxml_utils.hpp>
#include <boost/format.hpp>

#include <stdexcept>

using namespace rapidxml;
using boost::format;

namespace {
    template<typename T>
    std::string get_attr_required(T const& node, std::string const& attr_name) {
        xml_attribute<>* attr = node.first_attribute(attr_name.c_str());
        if (!attr) {
            throw std::runtime_error(str(format("Error parsing XML: required attribute %1% missing")
                % attr_name));
        }
        return attr->value();
    }

    int translate_fd(std::string const& fd_str) {
        if (fd_str == "stdin")
            return 0;
        if (fd_str == "stdout")
            return 1;
        if (fd_str == "stderr")
            return 2;

        int fd;
        namespace qi = boost::spirit::qi;
        std::string::const_iterator beg = fd_str.begin();
        std::string::const_iterator end = fd_str.end();
        if (qi::parse(beg, end, qi::int_, fd) && beg == end) {
            return fd;
        }

        throw std::runtime_error(str(format("Failed to parse file descriptor '%1%'") % fd_str));
    }
}

GraphXml::GraphXml(std::string const& path) {
    xml_document<> doc;
    std::string data = io::read_file(path);
    if (data.empty()) {
        throw std::runtime_error(str(format("Found no data in %1%. Is the file empty?") % path));
    }

    doc.parse<0>(&data[0]);

    xml_node<>* graph = doc.first_node("streamgraph");
    if (graph == 0) {
        throw std::runtime_error(str(format(
            "Error loading xml file %1%: expected <streamgraph> tag"
            ) % path));
    }

    char const* s = "command";
    for (xml_node<>* cmd = graph->first_node(s); cmd; cmd = cmd->next_sibling(s)) {
        parse_command(cmd);
    }

    s = "connect";
    for (xml_node<>* conn = graph->first_node(s); conn; conn = conn->next_sibling(s)) {
        parse_connection(conn);
    }

    s = "connect_input_file";
    for (xml_node<>* conn = graph->first_node(s); conn; conn = conn->next_sibling(s)) {
        parse_input_file_connection(conn);
    }

    s = "connect_output_file";
    for (xml_node<>* conn = graph->first_node(s); conn; conn = conn->next_sibling(s)) {
        parse_output_file_connection(conn);
    }
}

void GraphXml::parse_command(rapidxml::xml_node<>* cmd) {
    std::string name = get_attr_required(*cmd, "name");
    LOG(INFO) << "Parsing command " << name;
    xml_node<>* args = cmd->first_node("args");
    if (args == 0) {
        throw std::runtime_error(str(format(
            "Failed parsing <command> '%1%': expected nested <args> tag"
            ) % name));
    }

    std::vector<std::string> string_args;
    for (xml_node<>* arg = args->first_node("arg"); arg; arg = arg->next_sibling("arg")) {
        string_args.push_back(arg->value());
        LOG(INFO) << "\targ: " << string_args.back();
    }
    if (string_args.empty()) {
        throw std::runtime_error(str(format(
                "Failed parsing <command> '%1%' args: empty args array"
                ) % name));
    }

    std::pair<NameToId::iterator, bool> inserted = name_to_id_.insert(
        std::make_pair(name, 0));

    if (!inserted.second) {
        throw std::runtime_error(str(format(
            "duplicate command name '%1%'"
            ) % name));
    }

    ProcessGraph::NodeId id = graph_.add(string_args);
    inserted.first->second = id;
}


ProcessGraph::NodeId GraphXml::get_id(std::string const& name) const {
    NameToId::const_iterator iter = name_to_id_.find(name);
    if (iter == name_to_id_.end()) {
        throw std::runtime_error(str(format("Unknown name in connection: %1%") % name));
    }
    return iter->second;
}

void GraphXml::parse_connection(rapidxml::xml_node<>* conn) {
    std::string src = get_attr_required(*conn, "source");
    std::string src_fd_str = get_attr_required(*conn, "source_fd");
    int src_fd = translate_fd(src_fd_str);

    std::string dst = get_attr_required(*conn, "target");
    std::string dst_fd_str = get_attr_required(*conn, "target_fd");
    int dst_fd = translate_fd(dst_fd_str);

    graph_.connect(get_id(src), src_fd, get_id(dst), dst_fd);
}

void GraphXml::parse_input_file_connection(rapidxml::xml_node<>* conn) {
    std::string src_path = get_attr_required(*conn, "source");

    std::string dst = get_attr_required(*conn, "target");
    std::string dst_fd_str = get_attr_required(*conn, "target_fd");
    int dst_fd = translate_fd(dst_fd_str);

    graph_.connect_input_file(src_path, get_id(dst), dst_fd);
}

void GraphXml::parse_output_file_connection(rapidxml::xml_node<>* conn) {
    std::string src = get_attr_required(*conn, "source");
    std::string src_fd_str = get_attr_required(*conn, "source_fd");
    int src_fd = translate_fd(src_fd_str);

    std::string dst_path = get_attr_required(*conn, "target");

    graph_.connect_output_file(get_id(src), src_fd, dst_path);
}
