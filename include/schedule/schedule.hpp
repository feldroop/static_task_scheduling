#pragma once

#include <algorithm>
#include <optional>
#include <sstream>
#include <ranges>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <cluster/cluster.hpp>
#include <schedule/node_schedule.hpp>
#include <schedule/time_interval.hpp>
#include <util/epsilon_compare.hpp>
#include <util/timepoint.hpp>
#include <workflow/data_transfer_cost.hpp>
#include <workflow/workflow.hpp>

namespace schedule {

class schedule {
    bool use_memory_requirements;

    // cluster node id/index -> list of scheduled tasks
    std::vector<node_schedule> node_schedules{};
    std::unordered_map<workflow::task_id, std::vector<time_interval>> task_intervals{};
    std::unordered_map<scheduled_task_id, workflow::task_id> scheduled_to_original_task_id{};

public:
    struct scheduled_edge {
        workflow::task_id from_t_id;
        cluster::node_id from_n_id;
        workflow::task_id to_t_id;
        cluster::node_id to_n_id;
    };

    schedule(cluster::cluster const & c, bool const use_memory_requirements_) 
        : use_memory_requirements{use_memory_requirements_} {
        for (cluster::cluster_node const & node : c) {
            node_schedules.emplace_back(node);
        }
    }

    void insert_into_node_schedule(
        workflow::task_id const t_id,
        cluster::node_id const n_id,
        workflow::workflow const & w,
        bool const unscheduled_predecessors_allowed = false
    ) {
        workflow::task const & t = w.get_task(t_id);
        node_schedule & node_s = node_schedules.at(n_id);
        
        double const ready_time = task_ready_time(t_id, w, n_id, unscheduled_predecessors_allowed);
        
        auto slot = node_s.compute_earliest_finish_time(ready_time, t);

        util::timepoint const start = slot.eft - node_s.get_computation_time(t);
        scheduled_task_id const sched_t_id = scheduled_to_original_task_id.size();
        time_interval best_interval{start, slot.eft, sched_t_id, n_id};

        add_scheduled_task(t_id, best_interval);
        scheduled_to_original_task_id.insert({sched_t_id, t_id});
        node_s.insert(slot.it, best_interval);
    }

    cluster::node_id insert_into_best_eft_node_schedule(
        workflow::task_id const t_id,
        workflow::workflow const & w,
        bool const use_est_instead = false
    ) {
        workflow::task const & t = w.get_task(t_id);
        
        // if (t_id == 3) {
        //     double t0 = task_ready_time(t.id, w, 0);
        //     double eft0 = node_schedules.at(0).compute_earliest_finish_time(t0, t).eft;
        //     std::cout << "0: " << t0 << ", " << eft0 << '\n';
        //     double t1 = task_ready_time(t.id, w, 1);
        //     double eft1 = node_schedules.at(1).compute_earliest_finish_time(t1, t).eft;
        //     std::cout << "1: " << t1 << ", " << eft1 << '\n';
        //     double t2 = task_ready_time(t.id, w, 2);
        //     double eft2 = node_schedules.at(2).compute_earliest_finish_time(t2, t).eft;
        //     std::cout << "2: " << t2 << ", " << eft2 << '\n';
        // }

        auto has_enough_memory = [this, &t] (node_schedule const & node_s) {
            return use_memory_requirements 
                ? node_s.get_node().memory >= t.memory_requirement
                : true;
        };

        auto earliest_finish_time_of_node = [this, &w, &t] (node_schedule & node_s) {
            cluster::node_id const node_id = node_s.get_node().id;
            double const ready_time = task_ready_time(t.id, w, node_id);
            return std::make_tuple(node_s.compute_earliest_finish_time(ready_time, t), node_id);
        };

        auto earliest_finish_times = node_schedules
            | std::views::filter(has_enough_memory)
            | std::views::transform(earliest_finish_time_of_node);

        auto best_it = std::ranges::min_element(
            earliest_finish_times,
            {},
            [this, &t, use_est_instead] (auto const & tup) {
                auto const & node_s = node_schedules.at(std::get<1>(tup));
                // if (t.id == 3) {
                //     std::cout << std::get<1>(tup) << ", "
                //         << std::get<0>(tup).eft << ", "
                //         << (std::get<0>(tup).eft
                //         - use_est_instead ? node_s.get_computation_time(t) : 0.0) << '\n';
                // }
                return std::get<0>(tup).eft
                    - (use_est_instead ? node_s.get_computation_time(t) : 0.0);
            }
        );

        if (best_it == earliest_finish_times.end()) {
            throw std::logic_error(
                "There exists a task with a memory requirement larger than the memory of each node."
            );
        }

        auto const [slot, node_id] = *best_it;

        node_schedule & best_node_s = node_schedules.at(node_id);

        util::timepoint const start = slot.eft - best_node_s.get_computation_time(t);
        scheduled_task_id const sched_t_id = scheduled_to_original_task_id.size();
        time_interval best_interval{start, slot.eft, sched_t_id, node_id};

        add_scheduled_task(t_id, best_interval);
        scheduled_to_original_task_id.insert({sched_t_id, t_id});
        best_node_s.insert(slot.it, best_interval);

        return node_id;
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
            out << node_s.to_string(scheduled_to_original_task_id) << '\n';
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
            for (time_interval const & curr_t_interval : task_intervals.at(t.id)) {
                for (auto const & [predecessor_id, data_transfer] : w.get_task_incoming_edges(t.id)) {

                    auto const predecessor_interval_opt = find_predecessor_interval(
                        predecessor_id,
                        curr_t_interval,
                        data_transfer
                    );

                    if (!predecessor_interval_opt) {
                        return false;
                    }
                }
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
            for (time_interval const & curr_t_interval : task_intervals.at(t.id)) {
                cluster::node_id const & target_node_id = curr_t_interval.node_id;
                
                for (auto const & [predecessor_id, data_transfer] : w.get_task_incoming_edges(t.id)) {
                    auto const predecessor_interval_opt = find_predecessor_interval(
                        predecessor_id,
                        curr_t_interval,
                        data_transfer
                    );

                    cluster::node_id const & source_node_id = predecessor_interval_opt.value().node_id;

                    node_communication.at(source_node_id).at(target_node_id) += workflow::get_raw_data_transfer_cost(
                        data_transfer,
                        node_schedules.at(source_node_id).get_node().network_bandwidth
                    );
                } 
            }
        }

        return node_communication;
    }

    std::vector<workflow::task_id> get_tasks_of_node(cluster::node_id const n_id) const {
        std::vector<workflow::task_id> task_ids;
        std::vector<workflow::task_id> scheduled_task_ids = node_schedules.at(n_id)
            .get_scheduled_task_ids();

        for (scheduled_task_id const & sched_t_id : scheduled_task_ids) {
            task_ids.push_back(scheduled_to_original_task_id.at(sched_t_id));
        }

        return task_ids;
    }

    std::vector<scheduled_edge> get_different_node_edges(workflow::workflow const & w) const {
        std::vector<scheduled_edge> edges;
        
        for (auto const & [curr_t_id, curr_t_intervals] : task_intervals) {
            for (auto const & curr_t_interval : curr_t_intervals) {
                for (auto const & [pred_t_id, data_transfer] : w.get_task_incoming_edges(curr_t_id)) {
                    auto const pred_t_interval_opt = find_predecessor_interval(
                        pred_t_id,
                        curr_t_interval,
                        data_transfer
                    );

                    if (!pred_t_interval_opt) {
                        throw std::runtime_error("Internal Bug: Invalid schedule at end of TDCA.");
                    }
                    auto const pred_t_interval = pred_t_interval_opt.value();

                    if (curr_t_interval.node_id != pred_t_interval.node_id) {
                        edges.push_back({
                            pred_t_id,
                            pred_t_interval.node_id,
                            curr_t_id,
                            curr_t_interval.node_id
                        });
                    }
                }
            }
        }

        return edges;
    }

private:
    util::timepoint task_ready_time(
        workflow::task_id const t_id,
        workflow::workflow const & w,
        cluster::node_id const target_node_id,
        bool const unscheduled_predecessors_allowed = false
    ) const {
        auto data_available_times = w.get_task_incoming_edges(t_id)
            | std::views::transform([this, target_node_id, unscheduled_predecessors_allowed] (auto const & edge) {
                auto const & [predecessor_t_id, data_transfer] = edge;

                if (unscheduled_predecessors_allowed && !task_intervals.contains(predecessor_t_id)) {
                    return 0.0;
                }

                return get_earliest_data_available_time(predecessor_t_id, target_node_id, data_transfer);
            }             
        );

        auto latest_it = std::ranges::max_element(data_available_times);
        return latest_it != data_available_times.end() ? *latest_it : 0.0;
    }

    // due to possible duplication, find best connection to get the needed data
    util::timepoint get_earliest_data_available_time(
        workflow::task_id const predecessor_t_id,
        cluster::node_id const target_node_id,
        double const data_transfer
    ) const {
        auto const & intervals = task_intervals.at(predecessor_t_id);

        auto data_available_times = intervals 
            | std::views::transform([this, target_node_id, data_transfer] (time_interval const & interval) {
                return interval.end + workflow::get_data_transfer_cost(
                    interval.node_id, 
                    target_node_id,
                    data_transfer,
                    node_schedules.at(interval.node_id).get_node().network_bandwidth
                );
            });

        auto const earliest_it = std::ranges::min_element(data_available_times);

        if (earliest_it == data_available_times.end()) {
            throw std::runtime_error("Internal Bug: Predecessor task does not have any schedule yet.");
        }

        return *earliest_it;
    }

    void add_scheduled_task(workflow::task_id const t_id, time_interval const interval) {
        if (!task_intervals.contains(t_id)) {
            auto const [it, inserted] = task_intervals.insert({t_id, std::vector<time_interval>()});

            if (!inserted || it->first != t_id) {
                throw std::runtime_error("Internal bug: Task interval was not inserted correctly.");
            }
            
            it->second.push_back(interval);
        } else {
            task_intervals.at(t_id).push_back(interval);
        }
    }

    std::optional<time_interval> find_predecessor_interval(
        workflow::task_id const predecessor_id,
        time_interval const curr_t_interval,
        util::timepoint const data_transfer
    ) const {
        for (time_interval const & predecessor_interval : task_intervals.at(predecessor_id)) {
            double const data_transfer_cost = workflow::get_data_transfer_cost(
                curr_t_interval.node_id, 
                predecessor_interval.node_id, 
                data_transfer,
                node_schedules.at(curr_t_interval.node_id).get_node().network_bandwidth
            );

            // epsilon for floating point comparison
            if (util::epsilon_less_or_eq(predecessor_interval.end + data_transfer_cost, curr_t_interval.start)) {
                return predecessor_interval;
            }; 
        }

        return std::nullopt;
    }
};

} // namespace schedule
