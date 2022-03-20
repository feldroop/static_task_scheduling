#pragma once 

#include <cstddef>

namespace workflow {

struct task_bag {
    double const workload;
    double const input_data_size;
    double const output_data_size;
    size_t const memory_requirement;
    size_t const cardinality;
};

} // namespace workflow
