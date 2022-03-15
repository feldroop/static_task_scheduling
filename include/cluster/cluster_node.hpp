#pragma once

#include <cstddef>

namespace cluster {

struct cluster_node {
    size_t const id;
    size_t const memory;
    size_t const num_cores;
    double const core_performance;
    double const network_bandwidth;
};

} // namespace cluster
