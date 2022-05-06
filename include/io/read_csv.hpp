#pragma once

#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

// no threads needed as the input data sizes are usually not large
#define CSV_IO_NO_THREAD 
#include <csv.h>

#include <cluster/cluster.hpp>
#include <workflow/task_bag.hpp>
#include <workflow/task_dependency.hpp>

namespace io {

template <unsigned int I>
using MyCSVReader = CSVReader<
    I,
    trim_chars<' ', '\t'>,
    no_quote_escape<','>,
    throw_on_overflow,
    single_and_empty_line_comment<'#'>
>;

std::vector<cluster::cluster_node> read_cluster_csv(std::string const & filename) {
    std::vector<cluster::cluster_node> nodes;
    MyCSVReader<4> in(filename);

    in.read_header(ignore_no_column, "bandwidth", "performance", "memory", "num_cores");

    double bandwidth, performance, memory;
    size_t num_cores, id{0};

    while (in.read_row(bandwidth, performance, memory, num_cores)) {
        nodes.emplace_back(id, bandwidth, performance, memory, num_cores);
        ++id;
    }

    if (nodes.empty()) {
        throw std::runtime_error("Cluster must have at least 1 node.");
    }

    double const common_bandwidth = nodes.front().network_bandwidth;

    for (cluster::cluster_node const & node : nodes) {
        if (node.network_bandwidth != common_bandwidth) {
            std::cout << "WARNING: Not all cluster nodes have the same bandwidth\n";
            break;
        }
    }

    return nodes;
}

std::vector<workflow::task_bag> read_task_bag_csv(std::string const & filename) {
    std::vector<workflow::task_bag> task_bags;
    MyCSVReader<5> in(filename);

    in.read_header(ignore_no_column, "workload", "input_data_size", "output_data_size", "memory", "cardinality");

    double workload, input_data_size, output_data_size, memory;
    size_t cardinality;
    
    workflow::task_bag_id id{0};

    while (in.read_row(workload, input_data_size, output_data_size, memory, cardinality)) {
        task_bags.emplace_back(id, workload, input_data_size, output_data_size, memory, cardinality);
        ++id;
    }

    return task_bags;
}

std::vector<workflow::task_dependency> read_dependency_csv(std::string const & filename) {
    std::vector<workflow::task_dependency> dependencies;
    MyCSVReader<2> in(filename);

    in.read_header(ignore_no_column, "from_id", "to_id");

    size_t from_id, to_id;

    while (in.read_row(from_id, to_id)) {
        dependencies.emplace_back(from_id, to_id);
    }

    return dependencies;
}

std::vector<cluster::node_id> read_task_to_node_assignment_csv(
    std::string const & filename, 
    size_t const num_tasks,
    size_t const num_nodes
) {
    std::vector<cluster::node_id> task_to_node_assignment(num_tasks);
    
    // set to validate that all tasks got exactly one assignment
    auto range = std::views::iota(1ul, num_tasks + 1);
    std::unordered_set<size_t> expected_task_ids(range.begin(), range.end());

    MyCSVReader<3> in(filename);

    in.read_header(ignore_no_column, "task_number", "node_number", "is_assigned");

    size_t task_number, node_number, is_assigned;

    while (in.read_row(task_number, node_number, is_assigned)) {
        if (task_number > num_tasks) {
            throw std::runtime_error(
                "A task_number is larger than the expected maximum task number."
            );
        }

        if (node_number > num_nodes) {
            throw std::runtime_error(
                "A node_number is larger than the expected maximum task number."
            );
        }

        if (is_assigned > 1) {
            throw std::runtime_error("A value of the is_assinged column is neither 0 nor 1.");
        }

        if (is_assigned == 1) {
            if (!expected_task_ids.contains(task_number)) {
                throw std::runtime_error("A task has multiple assigned nodes.");
            }

            // the numbers are 1-based and internally we want to use 0-based ids
            task_to_node_assignment.at(task_number - 1) = node_number - 1;

            expected_task_ids.erase(task_number);
        }
    }

    if (!expected_task_ids.empty()) {
        throw std::runtime_error("Not all tasks were assigned a node.");
    }

    return task_to_node_assignment;
}

} // namespace io
