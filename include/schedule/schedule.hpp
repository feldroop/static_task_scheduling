#pragma once

#include <algorithm>
#include <iostream>
#include <ranges>
#include <tuple>
#include <vector>

#include <cluster.hpp>
#include <schedule/node_schedule.hpp>
#include <schedule/time_interval.hpp>
#include <workflow.hpp>

namespace schedule {

class schedule {
    // cluster node id/index -> list of scheduled tasks
    std::vector<node_schedule> nodes{};
    std::unordered_map<size_t, time_interval> task_intervals{};

public:
    schedule(cluster const & c) {
        for (cluster::cluster_node const & node : c) {
            nodes.emplace_back(node);
        }
    }

    void insert_into_best_eft_node_schedule(
        workflow::task_id const t_id,
        workflow const & w
    ) {
        workflow::task const & t = w.get_task(t_id);
        time_t const ready_time = task_ready_time(t_id, w);

        auto earliest_finish_times = nodes
            | std::views::filter([&t] (node_schedule const & node_s) {
                return node_s.get_node().memory >= t.memory_requirement;
            })
            | std::views::transform([&t, ready_time] (node_schedule & node_s) {
                cluster::node_id const node_id = node_s.get_node().id;
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

        if (best_eft_it == earliest_finish_times.end()) {
            throw std::logic_error(
                "There exists a task with a memory requirement larger than the memory of each node."
            );
        }

        auto const [slot, node_id] = *best_eft_it;

        node_schedule & best_node_s = nodes.at(node_id);
        time_t const start = slot.eft - best_node_s.get_computation_time(t);
        time_interval best_interval{start, slot.eft, t.id};

        task_intervals.insert({t.id, best_interval});
        nodes.at(node_id).insert(slot.it, best_interval);
    }

    time_t get_makespan() const {
        auto it = std::max_element(
            nodes.cbegin(), 
            nodes.cend(), 
            [] (auto const & node_s0, auto const & node_s1)
            {
                return node_s0.get_total_finish_time() < node_s1.get_total_finish_time();
            }
        );

        return it == nodes.end() ? 0.0 : it->get_total_finish_time();
    }

    void print() const {
        for (node_schedule const & node_s : nodes) {
            std::cout << node_s.to_string() << '\n';
        }

        std::cout << "[makespan: " << get_makespan() << "]" << std::endl;
    }

    bool is_valid(workflow const & w) const {
        for (workflow::task const & t : w) {
            for (auto const & [neighbor_id, weight] : w.get_task_incoming_edges(t.id)) {
                if (task_intervals.at(neighbor_id).end + weight > task_intervals.at(t.id).start) {
                    return false;
                }; 
            }
        }
        return true;
    }

private:
    time_t task_ready_time(
        workflow::task_id const t_id,
        workflow const & w
    ) const {
        auto data_available_times = w.get_task_incoming_edges(t_id)
            | std::views::transform([this] (auto const & edge) {
                auto const & [neighbor_id, weight] = edge;
                return task_intervals.at(neighbor_id).end + weight;
            }             
        );

        auto latest_it = std::max_element(data_available_times.begin(), data_available_times.end());
        return latest_it != data_available_times.end() ? *latest_it : 0.0;
    }
};

} // namespace schedule
