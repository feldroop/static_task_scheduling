#pragma once

#include <iostream>
#include <optional>

#include <io/command_line_arguments.hpp>

#include <clipp.h>

std::optional<command_line_arguments> parse_command_line(int argc, char *argv[]) {
    using namespace clipp;
    command_line_arguments args{};

    auto cluster_option = required("-c", "--cluster") & value("cluster_file", args.cluster_input);
    auto task_option = required("-t", "--tasks") & value("tasks_file", args.tasks_input);
    auto dependency_option = required("-d", "--dependencies") & value("dependencies_file", args.dependencies_input);
    auto workflow_option = required("-w", "--workflow") & value("workflow_file", args.tasks_input);

    auto output_option = option("-o", "--output") & value("output_file", args.output);
    auto verbose_option = option("-v", "--verbose")
        .set(args.verbose);

    // I am not 100% if it is allowed to use the same option multiple times, but it works
    auto cli = (
        cluster_option % "CLUSTER DOC",
        (
            (task_option % "TASK DOC") &
            (dependency_option % "DEPENDENCY DOC")
        ) |
        (
            task_option &
            (workflow_option % "WORKFLOW DOC")
        ) |
        workflow_option,
        output_option % "OUTPUT DOC",
        verbose_option % "VERBOSITY DOC"
    );

    auto res = parse(argc, argv, cli);

    if(res.any_error()) {
        auto fmt = doc_formatting{}
           .last_column(120);

        std::cout << make_man_page(cli, "static_task_scheduling", fmt);
        return std::nullopt;
    }

    return args;
}
