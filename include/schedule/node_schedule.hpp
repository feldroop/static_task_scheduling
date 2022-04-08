#pragma once

#include <algorithm>
#include <sstream>
#include <vector>

#include <cluster/cluster_node.hpp>
#include <schedule/time_interval.hpp>
#include <util/epsilon_compare.hpp>
#include <util/timepoint.hpp>
#include <workflow/workflow.hpp>

namespace schedule {

class node_schedule {
    std::vector<time_interval> intervals{};
    cluster::cluster_node const & node;

public:
    using iterator = std::vector<time_interval>::iterator;

    struct time_slot {
        util::timepoint const eft;
        iterator const it;
    };

    node_schedule(cluster::cluster_node const & node_) :
        node{node_} {}

    // returns EFT and iterator before which the task could be scheduled
    time_slot compute_earliest_finish_time(
        util::timepoint const ready_time,
        workflow::task const & t
    ) {
        auto ends_before = [] (time_interval const & interval, util::timepoint const & time) {
            return interval.end < time;
        };
        auto curr_it = std::lower_bound(intervals.begin(), intervals.end(), ready_time, ends_before);
        
        util::timepoint const computation_time = get_computation_time(t);

        if (curr_it == intervals.end()) {
            // no insertion possible -> schedule task to end after ready time
            util::timepoint const earliest_start_time = intervals.empty() ? ready_time : std::max(intervals.back().end, ready_time);
            return time_slot{earliest_start_time + computation_time, intervals.end()};
        }

        if (curr_it == intervals.begin() && curr_it->start >= ready_time + computation_time) {
            // insertion possible at ready time before any other task on this node
            return time_slot{ready_time + computation_time, curr_it};
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

    void insert(iterator const & it, time_interval const interval) {
        intervals.emplace(it, interval);
    }

    bool is_valid() const {
        if (intervals.empty()) {
            return true;
        }

        for (size_t i = 0; i < intervals.size(); ++i) {
            // interval itself must be consistent
            if (util::epsilon_greater(intervals[i].start, intervals[i].end)) {
                return false;
            }

            // interval must end before the next
            if (
                i < intervals.size() - 1 
                && util::epsilon_greater(intervals[i].end, intervals[i + 1].start)
            ) {
                return false;
            }
        }

        return true;
    }

    cluster::cluster_node const & get_node() const {
        return node;
    }

    util::timepoint get_computation_time(workflow::task const & t) const {
        return t.workload / node.performance();
    }

    util::timepoint get_total_finish_time() const {
        return intervals.empty() ? 0.0 : intervals.back().end;
    }

    std::string to_string() const {
        std::stringstream out;

        out << "Node " << node.id << ':';
        for (time_interval const & interval : intervals) {
            out << " (" << interval.task_id << ": " << interval.start
                << " -> " << interval.end << ")";  
        }

        return out.str();
    }
};

} // namespace schedule
