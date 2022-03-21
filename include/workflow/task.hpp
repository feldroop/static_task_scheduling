#pragma once

#include <cstddef>

namespace workflow {

using task_id = size_t;

struct task {
    task_id const id;
    double const workload;
    double const memory_requirement;
};

} // namespace workflow
