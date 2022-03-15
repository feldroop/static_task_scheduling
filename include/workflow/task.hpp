#pragma once

#include <cstddef>

namespace workflow {

struct task {
    size_t const id;
    double const compute_cost;
    size_t const memory_requirement;
};

} // namespace workflow
