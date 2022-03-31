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
    auto topology_option = required("-t", "--topology") & value("topology", args.topology);

    auto output_option = option("-o", "--output") & value("output_file", args.output);
    auto verbose_option = option("-v", "--verbose").set(args.verbose);

    auto use_memory_option = option("-m", "--use-memory-requirements").set(args.use_memory_requirements);

    std::string const cluster_doc = (
        "File in .csv format that describes the cluster architecture. "
        "It should contain exactly the fields bandwidth, performance, memory and num_cores."
    );
    std::string const task_bags_doc = (
        "File in .csv format that describes the tasks of the workflow. "
        "It should contain exactly the fields workload, input_data_size, output_data_size, "
        "memory and cardinality. "
    );
    std::string const dependency_doc = (
        "File in .csv format that contains the dependencies for the workflow tasks. " 
        "It should contain exactly the fields from_id and to_id."
    );
    std::string const topology_doc = (
        "Desired topology of the workflow. The dependencies will be inferred from the task bags "
        "using this configuration. Must be one of: epigenome, cybershake, ligo or montage."
    );
    std::string const output_doc = (
        "If given, the verbose output of this program is written to this file as plain text."
    );
    std::string const verbosity_doc = (
        "If given, all metrics and the full solution are printed to the command line."
    );
    std::string const use_memory_doc = (
        "If given, tasks are only scheduled onto cluster nodes with sufficient memory. "
        "This is not part of the original HEFT and CPOP and is deactivated by default."
    );

    auto cli = (
        "Input" % (
            cluster_option % cluster_doc,
            task_bags_option % task_bags_doc,
            (dependency_option % dependency_doc) | (topology_option % topology_doc)
        ),
        "Output" % (
            output_option % output_doc,
            verbose_option % verbosity_doc
        ),
        (use_memory_option % use_memory_doc)
    );

    auto res = parse(argc, argv, cli);

    if(res.any_error()) {
        std::cout << make_man_page(cli, "static_task_scheduling");
        return std::nullopt;
    }

    return args;
}

} // namespace io
