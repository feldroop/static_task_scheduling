#pragma once

// no threads needed as the input data sizes are usually not large
#define CSV_IO_NO_THREAD 
#include <csv.h>

#include <cluster/cluster.hpp>
#include <workflow/task_bag.hpp>
#include <workflow/dependency.hpp>

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

    return nodes;
}

std::vector<workflow::task_bag> read_task_bag_csv(std::string const & filename) {
    std::vector<workflow::task_bag> task_bags;
    MyCSVReader<5> in(filename);

    in.read_header(ignore_no_column, "workload", "input_data_size", "output_data_size", "memory", "cardinality");

    double workload, input_data_size, output_data_size, memory;
    size_t cardinality;

    while (in.read_row(workload, input_data_size, output_data_size, memory, cardinality)) {
        task_bags.emplace_back(workload, input_data_size, output_data_size, memory, cardinality);
    }

    return task_bags;
}

std::vector<workflow::dependency> read_dependency_csv(std::string const & filename) {
    std::vector<workflow::dependency> dependencies;
    MyCSVReader<2> in(filename);

    in.read_header(ignore_no_column, "from_id", "to_id");

    size_t from_id, to_id;

    while (in.read_row(from_id, to_id)) {
        dependencies.emplace_back(from_id, to_id);
    }

    return dependencies;
}

} // namespace io
