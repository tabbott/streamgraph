#pragma once

#include "process/Process.hpp"

#include <rapidxml.hpp>

#include <boost/lexical_cast.hpp>

#include <sys/resource.h>
#include <sys/types.h>

class StatusXml {
public:
    explicit StatusXml(std::vector<Process::Ptr> const& processes);

    void write(std::string const& path) const;

private:
    void add_process(Process::Ptr const& proc);
    rapidxml::xml_node<>* resource_node(rusage const& rsrc);

    template<typename T>
    rapidxml::xml_attribute<>* create_attr(std::string const& name, T const& value) {
        return doc_.allocate_attribute(doc_.allocate_string(name.c_str()),
            doc_.allocate_string(boost::lexical_cast<std::string>(value).c_str()));
    }

    template<typename T>
    rapidxml::xml_node<>* create_value_node(std::string const& name, T const& value) {
        auto node = doc_.allocate_node(rapidxml::node_element, doc_.allocate_string(name.c_str()));
        node->append_attribute(create_attr("value", value));
        return node;
    }

private:
    rapidxml::xml_document<> doc_;
    rapidxml::xml_node<>* root_;
};
