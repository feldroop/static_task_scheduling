#pragma once

#include <string>

namespace io {

struct command_line_arguments {
    std::string cluster_input{};
    std::string task_bag_input{};
    std::string dependency_input{};
    std::string topology{};

    std::string task_to_node_assignment_input{};

    std::string output{};
    bool verbose{false};

    bool use_memory_requirements{false};
};

} // namespace io
