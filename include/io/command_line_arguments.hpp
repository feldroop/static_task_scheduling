#pragma once

#include <string>

#include <clipp.h>

struct command_line_arguments {
    std::string cluster_input{};
    std::string task_bags_input{};
    std::string dependencies_input{};
    std::string workflow_input{};

    std::string output{};
    bool verbose{false};
};
