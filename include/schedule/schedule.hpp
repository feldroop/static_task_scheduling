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
#include <util/epsilon_compare.hpp>
#include <util/timepoint.hpp>
#include <workflow/workflow.hpp>

namespace schedule {

class schedule {
    bool use_memory_requirements;

    // cluster node id/index -> list of scheduled tasks
    std::vector<node_schedule> node_schedules{};
    std::unordered_map<size_t, time_interval> task_intervals{};

public:
    schedule(cluster::cluster const & c, bool const use_memory_requirements_) 
        : use_memory_requirements{use_memory_requirements_} {
        for (cluster::cluster_node const & node : c) {
            node_schedules.emplace_back(node);
        }
    }

    void insert_into_node_schedule(
        workflow::task_id const t_id,
        cluster::node_id const n_id,
        workflow::workflow const & w
    ) {
        workflow::task const & t = w.get_task(t_id);
        node_schedule & node_s = node_schedules.at(n_id);
        
        double const ready_time = task_ready_time(t.id, w, n_id);
        
        auto slot = node_s.compute_earliest_finish_time(ready_time, t);

        util::timepoint const start = slot.eft - node_s.get_computation_time(t);
        time_interval best_interval{start, slot.eft, t.id, n_id};

        task_intervals.insert({t.id, best_interval});
        node_s.insert(slot.it, best_interval);
    }

    void insert_into_best_eft_node_schedule(
        workflow::task_id const t_id,
        workflow::workflow const & w
    ) {
        workflow::task const & t = w.get_task(t_id);

        auto has_enough_memory = [this, &t] (node_schedule const & node_s) {
            if (use_memory_requirements) {
                return node_s.get_node().memory >= t.memory_requirement;
            } else {
                return true;
            }
        };

        auto earliest_finish_time_of_node = [this, &w, &t] (node_schedule & node_s) {
            cluster::node_id const node_id = node_s.get_node().id;
            double const ready_time = task_ready_time(t.id, w, node_id);
            return std::make_tuple(node_s.compute_earliest_finish_time(ready_time, t), node_id);
        };

        auto earliest_finish_times = node_schedules
            | std::views::filter(has_enough_memory)
            | std::views::transform(earliest_finish_time_of_node);

        auto best_eft_it = std::ranges::min_element(
            earliest_finish_times,
            {},
            [] (auto const & tup) {
                return std::get<0>(tup).eft;
            }
        );

        if (best_eft_it == earliest_finish_times.end()) {
            throw std::logic_error(
                "There exists a task with a memory requirement larger than the memory of each node."
            );
        }

        auto const [slot, node_id] = *best_eft_it;

        node_schedule & best_node_s = node_schedules.at(node_id);
        util::timepoint const start = slot.eft - best_node_s.get_computation_time(t);
        time_interval best_interval{start, slot.eft, t.id, node_id};

        task_intervals.insert({t.id, best_interval});
        best_node_s.insert(slot.it, best_interval);
    }

    util::timepoint get_makespan() const {
        auto it = std::ranges::max_element(
            node_schedules,
            {},
            [] (auto const & node_s)
            {
                return node_s.get_total_finish_time();
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
            out << "schedule " << (is_valid.value() ? "is " : "NOT ") << "valid\n";
        }

        out << '\n';

        return out.str();
    }

    bool is_valid(workflow::workflow const & w) const {
        for (node_schedule const & node_s : node_schedules) {
            if (!node_s.is_valid()) {
                return false;
            }
        }

        for (workflow::task const & t : w) {
            if (!task_intervals.contains(t.id)) {
                return false;
            }
        }
        
        for (workflow::task const & t : w) {
            time_interval const & curr_t_interval = task_intervals.at(t.id);
            
            for (auto const & [neighbor_id, data_transfer] : w.get_task_incoming_edges(t.id)) {
                time_interval const & neighbor_interval = task_intervals.at(neighbor_id);

                double const data_transfer_cost = get_data_transfer_cost(
                    curr_t_interval.node_id, 
                    neighbor_interval.node_id, 
                    data_transfer
                );

                // epsilon for floating point comparison
                if (util::epsilon_greater(neighbor_interval.end + data_transfer_cost, curr_t_interval.start)) {
                    return false;
                }; 
            }
        }
        return true;
    }

    // node_communication[source][target] -> total sum of data transfer from source to target
    std::vector<std::vector<double>> compute_node_communication_matrix(workflow::workflow const & w) const {
        std::vector<std::vector<double>> node_communication(
            node_schedules.size(),
            std::vector<double>(node_schedules.size(), 0.0)
        );
        
        for (workflow::task const & t : w) {
            cluster::node_id const & source_node_id = task_intervals.at(t.id).node_id;
            
            for (auto const & [neighbor_id, data_transfer] : w.get_task_outgoing_edges(t.id)) {
                cluster::node_id const & target_node_id = task_intervals.at(neighbor_id).node_id;

                node_communication[source_node_id][target_node_id] += get_raw_data_transfer_cost(
                    source_node_id, 
                    target_node_id, 
                    data_transfer
                );
            }
        }

        return node_communication;
    }

private:
    util::timepoint task_ready_time(
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

        auto latest_it = std::ranges::max_element(data_available_times);
        return latest_it != data_available_times.end() ? *latest_it : 0.0;
    }

    util::timepoint get_data_transfer_cost(
        cluster::node_id const node_id0,
        cluster::node_id const node_id1,
        double const data_transfer
    ) const {
        // if the two tasks are scheduled to the same node, there is no cost for the data transfer
        if (node_id0 == node_id1) {
            return 0.0;
        }

        return get_raw_data_transfer_cost(node_id0, node_id1, data_transfer);
    }

    // data transfer cost without taking into account equal node ids
    util::timepoint get_raw_data_transfer_cost(
        cluster::node_id const node_id0,
        [[maybe_unused]] cluster::node_id const node_id1,
        double const data_transfer
    ) const {
        // for now, just use the bandwidth of node0 as they are assumed to be equal
        double const bandwidth = node_schedules.at(node_id0).get_node().network_bandwidth;

        return data_transfer / bandwidth;
    }
};

} // namespace schedule
