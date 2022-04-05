#pragma once 

#include <cstddef>

namespace workflow {

using task_bag_id = size_t;

struct task_bag {
    task_bag_id const id;
    double const workload;
    double const input_data_size;
    double const output_data_size;
    double const memory_requirement;
    size_t const cardinality;
};

} // namespace workflow
