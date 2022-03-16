#pragma once

#include <vector>

#include <cluster.hpp>
#include <schedule/schedule.hpp>
#include <workflow.hpp>

namespace algorithms {

// Running time analysis:
// input: cluster C, workflow-DAG W = (V,E)
//
// compute_mean_performance: mean of all cluster nodes -> O(|C|)
// compute_all_upward_ranks: visit all edges in topological order -> O(|E|)
// sort_task_ids_by_rank: sort list of size |V| -> O(|V| * log(|V|))
// compute_ready_time: visit all incoming edges once -> in total O(|E|) 
// |V| times insert_into_best_eft_node_schedule:
//    |C| times compute_earliest_finish_time:
//        up to |V| if all tasks are scheduled to the beginning (in practice rather O(log(|V|))
//    once insert into node_schedule: up to |V|, but in practice rather O(1)
//
// in total: O(|V|^2 * |C|) worst case, however in practice usually O(|V| * log(|V|) * |C|) or O(|E|)
// this implementation is in some cases asymptotically slower than the suggested running time
// in the original paper which is O(|E| * |C|)

schedule::schedule heft(cluster const & c, workflow const & w) {
    // instead of computing average compute cost for every task,
    // multiply given costs by average cluster node performance
    double const mean_cluster_performance = c.mean_node_performance();

    std::vector<size_t> const priority_list = w.task_ids_sorted_by(
        workflow::task_order::upward_rank, 
        mean_cluster_performance
    );

    schedule::schedule s(c);

    for (workflow::task_id const t_id : priority_list) {
        s.insert_into_best_eft_node_schedule(t_id, w);
    }
    
    return s;
}

} // namespace algorithms
