#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <ranges>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <cluster/cluster.hpp>
#include <schedule/schedule.hpp>
#include <util/topological_order.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {

double compute_mean_performance(cluster::cluster const & c) {
    double const performance_sum = std::transform_reduce(
        c.cbegin(), 
        c.cend(),
        0.0,
        std::plus<>(),
        [] (auto const & n) {
            return n.core_performance * n.num_cores;
        }
    );

    return performance_sum / c.size();
}

double compute_upward_rank(
    workflow::workflow const & w,
    std::unordered_map<size_t, double> const & upward_ranks,
    double const mean_performance,
    size_t const id
) {
    double upward_rank = w.get_vertex(id).compute_cost * mean_performance;

    auto outgoing_ranks = w.get_outgoing_edges(id) 
        | std::views::transform([&upward_ranks] (auto const & edge) {
            auto const & [neighbor_id, weight] = edge;
            return weight + upward_ranks.at(neighbor_id);
        }
    );

    auto const max_it = std::max_element(outgoing_ranks.begin(), outgoing_ranks.end());
    if (max_it != outgoing_ranks.end()) {
        upward_rank += *max_it;
    }

    return upward_rank;
}

std::unordered_map<size_t, double> compute_all_upward_ranks(workflow::workflow const & w, double const mean_performance) {
    std::unordered_map<size_t, double> upward_ranks{};
    
    auto topological_order = util::compute_topological_order(w);
    for (size_t const id : std::views::reverse(topological_order))
    {
        upward_ranks[id] = compute_upward_rank(w, upward_ranks, mean_performance, id);
    }

    return upward_ranks;
}

std::vector<size_t> sort_task_ids_by_rank(std::unordered_map<size_t, double> const & upward_ranks) {
    std::vector<size_t> priority_list(upward_ranks.size());
    std::iota(priority_list.begin(), priority_list.end(), 0);

    std::ranges::sort(priority_list, 
        [&upward_ranks] (size_t const & id0, size_t const & id1) {
            return upward_ranks.at(id0) > upward_ranks.at(id1);
        }
    );

    return priority_list;
}

double compute_ready_time(
    std::unordered_map<size_t, double> const & incoming_edges,
    schedule::schedule const & s
) {
    auto data_available_times = incoming_edges
        | std::views::transform([&s] (auto const & edge) {
            auto const & [neighbor_id, weight] = edge;
            return s.get_task_interval(neighbor_id).end + weight;
        }             
    );

    auto latest_it = std::max_element(data_available_times.begin(), data_available_times.end());
    return latest_it != data_available_times.end() ? *latest_it : 0.0;
}

void insert_into_best_eft_node_schedule(
    schedule::schedule & s,
    workflow::task const & t,
    double const ready_time
) {
    auto earliest_finish_times = s
        | std::views::filter([&t] (schedule::node_schedule const & node_s) {
            return node_s.get_node().memory >= t.memory_requirement;
        })
        | std::views::transform([&t, ready_time] (schedule::node_schedule & node_s) {
            size_t const node_id = node_s.get_node().id;
            return std::make_tuple(node_s.compute_earliest_finish_time(ready_time, t), node_id);
        }
    );

    auto best_eft_it = std::min_element(
        earliest_finish_times.begin(), 
        earliest_finish_times.end(),
        [] (auto const & tup0, auto const & tup1) {
            return std::get<0>(tup0).eft < std::get<0>(tup1).eft;
        }
    );

    assert(best_eft_it != earliest_finish_times.end());
    auto const [slot, node_id] = *best_eft_it;

    schedule::node_schedule & best_node = s.at(node_id);
    double const start = slot.eft - best_node.get_computation_time(t);
    schedule::time_interval best_interval{start, slot.eft, t.id};

    s.add_task_interval(t.id, best_interval);
    s.at(node_id).insert(slot.it, best_interval);
}

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

schedule::schedule heft(cluster::cluster const & c, workflow::workflow const & w) {
    // instead of computing average compute cost for every task,
    // multiply given costs by average cluster node performance
    double const mean_performance = compute_mean_performance(c);

    std::unordered_map<size_t, double> const upward_ranks = compute_all_upward_ranks(w, mean_performance);

    std::vector<size_t> const priority_list = sort_task_ids_by_rank(upward_ranks);

    schedule::schedule s(c);

    for (size_t const task_id : priority_list) {
        double const ready_time = compute_ready_time(w.get_incoming_edges(task_id), s);

        insert_into_best_eft_node_schedule(s, w.get_vertex(task_id), ready_time);
    }
    
    return s;
}

} // namespace algorithms
