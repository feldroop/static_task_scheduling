#pragma once

#include <iostream>
#include <optional>

#include <io/command_line_arguments.hpp>

#include <clipp.h>

namespace io {

std::optional<command_line_arguments> parse_command_line(int argc, char *argv[]) {
    using namespace clipp;
    command_line_arguments args{};

    auto cluster_option = required("-c", "--cluster") & value("cluster_file", args.cluster_input);
    auto task_bags_option = required("-t", "--tasks") & value("tasks_file", args.task_bag_input);
    auto dependency_option = required("-d", "--dependencies") & value("dependencies_file", args.dependency_input);
    auto workflow_option = required("-w", "--workflow") & value("workflow_file", args.task_bag_input);

    auto output_option = option("-o", "--output") & value("output_file", args.output);
    auto verbose_option = option("-v", "--verbose")
        .set(args.verbose);

    std::string const cluster_doc = (
        "File in .csv format that describes the cluster architecture. "
        "It should contain exactly the fields bandwidth, performance, memory and num_cores."
    );
    std::string const task_bags_doc = (
        "File in .csv format that describes the tasks of the workflow. "
        "It should contain exactly the fields workload, input_data_size, output_data_size, "
        "memory and cardinality. "
        "This is used preferably over task descriptions in a workflow file and the tasks are "
        "assigned ids in ascending order after generating them from the bags."
    );
    std::string const dependency_doc = (
        "File in .csv format that describes the task dependencies. "
        "It should contain exactly the fields from_id and to_id."
    );
    std::string const workflow_doc = (
        "An XML file according to the schema at https://pegasus.isi.edu/schema/dax-2.1.xsd. "
        "If only this is given, the whole workflow is read from this file. If additionally tasks "
        "are supplied in a .csv file, those tasks are used instead."
    );
    std::string const output_doc = (
        "If given, the verbose output of this program is written to this file as plain text."
    );
    std::string const verbosity_doc = (
        "If given, all metrics and the full solution are printed to the command line."
    );

    // I am not 100% if it is allowed to use the same option multiple times, but it works
    auto cli = (
        "Input" % (
            cluster_option % cluster_doc,
            (
                (task_bags_option % task_bags_doc) &
                (dependency_option % dependency_doc)
            ) |
            (
                task_bags_option &
                (workflow_option % workflow_doc)
            ) |
            workflow_option
        ),
        "Output" % (
            output_option % output_doc,
            verbose_option % verbosity_doc
        )
    );

    auto res = parse(argc, argv, cli);

    if(res.any_error()) {
        auto fmt = doc_formatting{}
           .last_column(111);

        std::cout << make_man_page(cli, "static_task_scheduling", fmt);
        return std::nullopt;
    }

    return args;
}

} // namespace io
