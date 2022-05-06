#pragma once

#include <ranges>
#include <vector>

#include <cluster/cluster.hpp>
#include <cluster/cluster_node.hpp>
#include <schedule/schedule.hpp>
#include <workflow/task.hpp>
#include <workflow/workflow.hpp>

namespace schedule {

schedule from_assignment(
    std::vector<cluster::node_id> const & assignment,
    cluster::cluster const & c,
    workflow::workflow const & w,
    bool const use_memory_requirements
) {
    schedule s(c, use_memory_requirements);

    for (workflow::task_id t_id : std::views::iota(0ul, w.size())) {
        s.insert_into_node_schedule(t_id, assignment.at(t_id), w);
    }

    return s;
}

} //namespace schedule
