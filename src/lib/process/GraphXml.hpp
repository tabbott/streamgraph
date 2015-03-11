#pragma once

#include "process/ProcessGraph.hpp"

#include <rapidxml.hpp>
#include <boost/unordered_map.hpp>

#include <string>

class GraphXml {
public:
    explicit GraphXml(std::string const& path);

    ProcessGraph& graph() {
        return graph_;
    }

private:
    ProcessGraph::NodeId get_id(std::string const& name) const;

    void parse_command(rapidxml::xml_node<>* cmd);
    void parse_connection(rapidxml::xml_node<>* conn);
    void parse_input_file_connection(rapidxml::xml_node<>* conn);
    void parse_output_file_connection(rapidxml::xml_node<>* conn);


private:
    typedef boost::unordered_map<std::string, ProcessGraph::NodeId> NameToId;
    ProcessGraph graph_;
    NameToId name_to_id_;
};
