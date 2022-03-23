#pragma once

#include <algorithm>
#include <optional>
#include <sstream>
#include <ranges>
#include <tuple>
#include <vector>

#include <cluster/cluster.hpp>
#include <schedule/node_schedule.hpp>
#include <schedule/time_interval.hpp>
#include <workflow/workflow.hpp>

namespace schedule {

class schedule {
    // cluster node id/index -> list of scheduled tasks
    std::vector<node_schedule> node_schedules{};
    std::unordered_map<size_t, time_interval> task_intervals{};

public:
    schedule(cluster const & c) {
        for (cluster::cluster_node const & node : c) {
            node_schedules.emplace_back(node);
        }
    }

    void insert_into_best_eft_node_schedule(
        workflow::task_id const t_id,
        workflow::workflow const & w
    ) {
        workflow::task const & t = w.get_task(t_id);

        auto has_enough_memory = [&t] (node_schedule const & node_s) {
            return node_s.get_node().memory >= t.memory_requirement;
        };

        auto earliest_finish_time_of_node = [this, &w, &t] (node_schedule & node_s) {
            cluster::node_id const node_id = node_s.get_node().id;
            double const ready_time = task_ready_time(t.id, w, node_id);
            return std::make_tuple(node_s.compute_earliest_finish_time(ready_time, t), node_id);
        };

        auto earliest_finish_times = node_schedules
            | std::views::filter(has_enough_memory)
            | std::views::transform(earliest_finish_time_of_node);

        auto earlier_finish_time = [] (auto const & tup0, auto const & tup1) {
            return std::get<0>(tup0).eft < std::get<0>(tup1).eft;
        };

        auto best_eft_it = std::min_element(
            earliest_finish_times.begin(), 
            earliest_finish_times.end(),
            earlier_finish_time
        );

        if (best_eft_it == earliest_finish_times.end()) {
            throw std::logic_error(
                "There exists a task with a memory requirement larger than the memory of each node."
            );
        }

        auto const [slot, node_id] = *best_eft_it;

        node_schedule & best_node_s = node_schedules.at(node_id);
        time_t const start = slot.eft - best_node_s.get_computation_time(t);
        time_interval best_interval{start, slot.eft, t.id, node_id};

        task_intervals.insert({t.id, best_interval});
        node_schedules.at(node_id).insert(slot.it, best_interval);
    }

    time_t get_makespan() const {
        auto it = std::max_element(
            node_schedules.begin(), 
            node_schedules.end(), 
            [] (auto const & node_s0, auto const & node_s1)
            {
                return node_s0.get_total_finish_time() < node_s1.get_total_finish_time();
            }
        );

        return it == node_schedules.end() ? 0.0 : it->get_total_finish_time();
    }

    std::string to_string(std::string const & algo, std::optional<bool> const is_valid = std::nullopt) const {
        std::stringstream out;
        
        out << "########## " << algo << " Schedule: ##########\n";
        for (node_schedule const & node_s : node_schedules) {
            out << node_s.to_string() << '\n';
        }

        out << "makespan: " << get_makespan() << '\n';

        if (is_valid.has_value()) {
            out << "schedule " << (is_valid.value() ? "is " : "not ") << "valid\n";
        }

        return out.str();
    }

    bool is_valid(workflow::workflow const & w, double const epsilon = 0.0000000001) const {
        for (workflow::task const & t : w) {
            for (auto const & [neighbor_id, data_transfer] : w.get_task_incoming_edges(t.id)) {
                time_interval const & curr_t_interval = task_intervals.at(t.id);
                time_interval const & neighbor_interval = task_intervals.at(neighbor_id);

                double const data_transfer_cost = get_data_transfer_cost(
                    curr_t_interval.node_id, 
                    neighbor_interval.node_id, 
                    data_transfer
                );

                // epsilon added for floating point comparison
                if (neighbor_interval.end + data_transfer_cost > curr_t_interval.start + epsilon) {
                    return false;
                }; 
            }
        }
        return true;
    }

private:
    time_t task_ready_time(
        workflow::task_id const t_id,
        workflow::workflow const & w,
        cluster::node_id const target_node_id
    ) const {
        auto data_available_times = w.get_task_incoming_edges(t_id)
            | std::views::transform([this, target_node_id] (auto const & edge) {
                auto const & [neighbor_task_id, data_transfer] = edge;

                time_interval const & neighbor_interval = task_intervals.at(neighbor_task_id);

                return neighbor_interval.end + get_data_transfer_cost(
                    neighbor_interval.node_id, 
                    target_node_id,
                    data_transfer
                );
            }             
        );

        auto latest_it = std::max_element(data_available_times.begin(), data_available_times.end());
        return latest_it != data_available_times.end() ? *latest_it : 0.0;
    }

    time_t get_data_transfer_cost(
        cluster::node_id const node_id0,
        cluster::node_id const node_id1,
        double const data_transfer
    ) const {
        // if the two tasks are scheduled to the same node, there is no cost for the data transfer
        if (node_id0 == node_id1) {
            return 0.0;
        }

        double const node_bandwidth0 = node_schedules.at(node_id0).get_node().network_bandwidth;
        double const node_bandwidth1 = node_schedules.at(node_id1).get_node().network_bandwidth;

        double const common_bandwidth = std::min(node_bandwidth0, node_bandwidth1);

        return data_transfer / common_bandwidth;
    }
};

} // namespace schedule
