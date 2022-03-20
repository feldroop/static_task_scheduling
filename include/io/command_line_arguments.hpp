#pragma once

#include <string>

#include <clipp.h>

namespace io {

struct command_line_arguments {
    std::string cluster_input{};
    std::string task_bag_input{};
    std::string dependency_input{};
    std::string workflow_input{};

    std::string output{};
    bool verbose{false};
};

} // namespace io
