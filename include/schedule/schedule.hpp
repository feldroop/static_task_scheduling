#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#include <cluster/cluster.hpp>
#include <schedule/node_schedule.hpp>
#include <schedule/time_interval.hpp>
#include <workflow/workflow.hpp>

namespace schedule {

class schedule {
    // cluster node id/index -> list of scheduled tasks
    std::vector<node_schedule> nodes{};
    std::unordered_map<size_t, time_interval> task_intervals{};

public:
    using iterator = std::vector<node_schedule>::iterator;

    schedule(cluster::cluster const & c) {
        for (cluster::cluster_node const & node : c) {
            nodes.emplace_back(node);
        }
    }

    double get_makespan() const {
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

    bool is_valid(workflow::workflow const & w) {
        // TODO
        return true;
    }

    time_interval get_task_interval(size_t const task_id) const {
        return task_intervals.at(task_id);
    }

    void add_task_interval(size_t const task_id, time_interval const interval) {
        task_intervals.insert({task_id, interval});
    }

    iterator begin() {
        return nodes.begin();
    }

    iterator end() {
        return nodes.end();
    }

    node_schedule & at(size_t const index) {
        return nodes.at(index);
    }
};

} // namespace schedule
