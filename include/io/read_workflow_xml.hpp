#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>

#include <workflow/dependency.hpp>

#include <pugixml.hpp>

namespace io {

std::vector<workflow::dependency> read_workflow_xml(std::string const & filename) {
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file(filename.c_str());
    if (!result) {
        throw std::runtime_error("Could not open and parse the file " + filename);
    }

    std::unordered_map<std::string, size_t> to_internal_id;

    size_t id{0};
    for (pugi::xml_node const & task : doc.child("adag").children("job")) {
        to_internal_id.insert({task.attribute("id").as_string(), id});

        ++id;
    }

    std::vector<workflow::dependency> dependencies;

    for (pugi::xml_node const & child : doc.child("adag").children("child")) {
        std::string child_id = child.attribute("ref").as_string();
        for (pugi::xml_node const & parent : child.children("parent")) {
            std::string parent_id = parent.attribute("ref").as_string();
            dependencies.push_back({to_internal_id.at(parent_id), to_internal_id.at(child_id)});
        }
    }

    return dependencies;
}

} // namespace io
