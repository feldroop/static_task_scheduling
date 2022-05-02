#pragma once

#include <algorithm>
#include <vector>

#include <algorithms/common_clustering_based.hpp>
#include <cluster/cluster.hpp>
#include <io/command_line_arguments.hpp>
#include <io/issue_warning.hpp>
#include <schedule/schedule.hpp>
#include <workflow/task.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {

// in our model all tasks in a level/bag have the same workload
// hence, we only have to distribute the tasks evenly
std::vector<task_group> runtime_balanced_task_groups(
    workflow::workflow const & w,
    std::vector<workflow::task_id> const & bag, 
    size_t const num_cluster_nodes
) {
    size_t const num_groups = std::min(bag.size(), num_cluster_nodes);
    std::vector<task_group> groups(num_groups);

    auto const group_sizes = split_most_evenly(bag.size(), num_groups);

    size_t bag_index{0}, i{0};

    for (task_group & group : groups) {
        size_t const group_size = group_sizes.at(i);

        for (size_t j = 0; j < group_size; ++j) {
            workflow::task_id const t_id = bag.at(bag_index + j);
            group.add_task(w.get_task(t_id));
        }

        bag_index += group_size;
        ++i;
    }

    return groups;
}

// Runtime balance clustering algorithm

// Running time analysis:
// TODO

schedule::schedule rbca(
    cluster::cluster const & c, 
    workflow::workflow const & w,
    io::command_line_arguments const & args
) {
    schedule::schedule s(c, args.use_memory_requirements);

    if (args.use_memory_requirements) {
        io::issue_warning(args, "Memory requirements not implemented/used for RBCA");
    }

    // we use our bags instead of the levels as defined in the original paper 
    // (makes sense, but is not always equal)
    auto const & task_ids_per_bag = w.get_task_ids_per_bag();

    for (auto const & bag: task_ids_per_bag) {
        auto groups = runtime_balanced_task_groups(w, bag, c.size());
        select_good_processors_for_expensive_groups(
            c, w, s, groups, args.use_memory_requirements
        );
    }

    return s;
}

} // namespace algorithms
