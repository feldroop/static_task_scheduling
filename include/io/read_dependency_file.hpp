#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <io/read_csv.hpp>
#include <io/read_workflow_xml.hpp>
#include <workflow/task_dependency.hpp>

namespace io {

std::vector<workflow::task_dependency> read_dependency_file(std::string const & filename) {
    std::filesystem::path const dependency_path(filename);

    if (dependency_path.extension() == ".csv") {
        return io::read_dependency_csv(filename);
    } else if (dependency_path.extension() == ".xml") {
        return io::read_workflow_xml(filename);
    } else {
        throw std::runtime_error("Could not infer file type of dependency file.");
    }
}


} // namespace io
