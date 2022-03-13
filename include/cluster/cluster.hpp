#pragma once

#include <cassert>
#include <vector>

#include <cluster/cluster_node.hpp>

using cluster = std::vector<cluster_node>;

cluster create_cluster(
    std::vector<size_t> const & memories,
    std::vector<size_t> const & num_cores,
    std::vector<double> const & core_performances,
    std::vector<double> const & bandwidths
) {
    cluster c{};

    assert(
        memories.size() == num_cores.size() &&
        memories.size() == core_performances.size() &&
        memories.size() == bandwidths.size()
    );

    for (size_t i = 0; i < memories.size(); i++)
    {
        cluster_node n{memories[i], num_cores[i], core_performances[i], bandwidths[i]};
        c.push_back(n);
    }
    
    return c;
}