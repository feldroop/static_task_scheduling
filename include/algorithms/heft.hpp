#pragma once

#include <vector>

#include <cluster/cluster.hpp>
#include <schedule/schedule.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {

std::vector<workflow::task_id> task_ids_sorted_by_upward_ranks(
    std::unordered_map<size_t, double> const & upward_ranks
) {
    std::vector<workflow::task_id> priority_list(upward_ranks.size());
    std::iota(priority_list.begin(), priority_list.end(), 0);

    std::ranges::sort(priority_list, 
        [&upward_ranks] (workflow::task_id const & t_id0, workflow::task_id const & t_id1) {
            return upward_ranks.at(t_id0) > upward_ranks.at(t_id1);
        }
    );

    return priority_list;
}

// Running time analysis:
// input: cluster C, workflow-DAG W = (V,E)
// O(|V|^2 * |C|) worst case, however in practice ofter O(|V| * log(|V|) * |C|) or O(|E| * |C|)
// this implementation is in some cases asymptotically slower than the suggested
// running time in the original paper which is O(|E| * |C|)

schedule::schedule heft(
    cluster::cluster const & c, 
    workflow::workflow const & w
) {
    auto const upward_ranks = w.all_upward_ranks(
        c.mean_node_performance(),
        c.mean_node_bandwidth()
    );

    std::vector<size_t> const priority_list = task_ids_sorted_by_upward_ranks(upward_ranks);
    schedule::schedule s(c);

    for (workflow::task_id const t_id : priority_list) {
        s.insert_into_best_eft_node_schedule(t_id, w);
    }
    
    return s;
}

} // namespace algorithms
