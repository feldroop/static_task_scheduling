#pragma once

#include <algorithm>
#include <optional>
#include <sstream>
#include <tuple>
#include <vector>

#include <cluster/cluster_node.hpp>
#include <schedule/time_interval.hpp>
#include <workflow/task.hpp>

namespace schedule {

class node_schedule {
    std::vector<time_interval> intervals{};
    cluster::cluster_node const & node;

public:
    using iterator = std::vector<time_interval>::iterator;

    struct time_slot {
        double const eft;
        iterator const it;
    };

    node_schedule(cluster::cluster_node const & node_) :
        node{node_} {}

    // returns EFT and iterator before which the task could be scheduled
    time_slot compute_earliest_finish_time(
        double const ready_time,
        workflow::task const & t
    ) {
        auto ends_before = [] (time_interval const & interval, double const & time) {
            return interval.end < time;
        };
        auto curr_it = std::lower_bound(intervals.begin(), intervals.end(), ready_time, ends_before);
        
        double const computation_time = get_computation_time(t);

        if (curr_it == intervals.end()) {
            // no insertion possible -> schedule task to end after ready time
            double const earliest_start_time = intervals.empty() ? ready_time : std::max(intervals.back().end, ready_time);
            return time_slot{earliest_start_time + computation_time, intervals.end()};
        }

        if (curr_it == intervals.begin() && curr_it->start >= ready_time + computation_time) {
            // insertion possible at ready time before any other task on this node
            return time_slot{computation_time, curr_it};
        }

        while (true) {
            auto const next_it = curr_it + 1;

            if (
                next_it == intervals.end() || // no insertion possible -> schedule task to end
                next_it->start - curr_it->end >= computation_time // insertion possible here
            ) {
                return time_slot{curr_it->end + computation_time, next_it};
            }

            curr_it = next_it;
        }
    }

    void insert(
        iterator const & it,
        time_interval const interval
    ) {
        intervals.emplace(it, std::move(interval));
    }

    cluster::cluster_node const & get_node() const {
        return node;
    }

    double get_computation_time(workflow::task const & t) const {
        return t.compute_cost / (node.core_performance * node.num_cores);
    }

    double get_total_finish_time() const {
        return intervals.empty() ? 0.0 : intervals.back().end;
    }

    std::string to_string() const {
        std::stringstream out{};

        out << "[Node " << node.id << "]";
        for (time_interval const & interval : intervals) {
            out << " (" << interval.task_id << ": " << interval.start
                << " -> " << interval.end << ")";  
        }

        return out.str();
    }
};

} // namespace schedule
