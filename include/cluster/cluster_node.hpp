#pragma once

#include <cstddef>

struct cluster_node {
    size_t const memory;
    size_t const num_cores;
    double const core_performance;
    double const network_bandwidth;
};
