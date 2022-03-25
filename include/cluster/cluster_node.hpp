#pragma once

#include <string>
#include <sstream>

namespace cluster {

using node_id = size_t;

struct cluster_node {
    node_id const id;
    double const network_bandwidth;
    double const core_performance;
    double const memory;
    size_t const num_cores;

    double performance() const {
        // assumes perfectly parallelizable tasks
        return core_performance * num_cores;
    }

    std::string to_string() const {
        std::stringstream out{};

        out << "Node " << id
            << ": bandwidth " << network_bandwidth
            << ", performance " << core_performance
            << ", memory " << memory
            << ", num_cores " << num_cores;

        return out.str();
    }
};
} //namepsace cluster
