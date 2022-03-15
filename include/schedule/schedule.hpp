#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#include <cluster/cluster.hpp>
#include <schedule/node_schedule.hpp>

namespace schedule {

// cluster node id/index -> list of scheduled tasks
using schedule = std::vector<node_schedule>;

schedule create_empty_schedule(cluster::cluster const & c) {
    schedule s{};

    for (cluster::cluster_node const & node : c) {
        s.emplace_back(node);
    }

    return s;
}

void print_schedule(schedule const & s) {
    double makespan = 0.0;
    for (node_schedule const & curr : s) {
        std::cout << curr.to_string() << '\n';
        makespan = std::max(makespan, curr.get_total_finish_time());
    }

    std::cout << "[makespan: " << makespan << "]" << std::endl;
}

// TODO is_valid(workflow) -> make class, add task_intervals member

} // namespace schedule
