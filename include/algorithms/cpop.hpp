#pragma once 

#include <algorithm>
#include <queue>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cluster/cluster.hpp>
#include <schedule/schedule.hpp>
#include <util/epsilon_compare.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {

std::unordered_map<workflow::task_id, double> compute_task_priorities(
    std::unordered_map<workflow::task_id, double> downward_ranks,
    std::unordered_map<workflow::task_id, double> upward_ranks
) {
    std::unordered_map<workflow::task_id, double> task_priorities{};

    for (auto const & [t_id, downward_rank] : downward_ranks) {
        task_priorities.insert({t_id, downward_rank + upward_ranks.at(t_id)});
    }

    return task_priorities;
}

std::unordered_set<workflow::task_id> compute_critical_path(
    workflow::workflow const & w,
    std::unordered_map<workflow::task_id, double> task_priorities
) {
    // we don't enforce a single entry task and choose the independent task with the highest priority
    auto const & independent_task_ids = w.get_independent_task_ids();
    auto const max_it = std::ranges::max_element(independent_task_ids, {}, [&task_priorities] (workflow::task_id const & t_id) {
        return task_priorities.at(t_id);
    });
    // safe dereference because it is enforced that independent tasks exist
    double const critical_path_priority = task_priorities.at(*max_it);
    workflow::task_id curr_task_id = *max_it;

    std::unordered_set<workflow::task_id> critical_path{};
    auto const is_on_critical_path = [&task_priorities, critical_path_priority] (workflow::task_id const & t_id) {
        return util::epsilon_eq(task_priorities.at(t_id), critical_path_priority);
    };

    while (true) {
        critical_path.insert(curr_task_id);

        auto const successor_tasks = w.get_task_outgoing_edges(curr_task_id)
            | std::views::transform([] (auto const & edge) {
                auto const & [neighbor_id, weight] = edge;
                return neighbor_id;
            });

        auto const next_it = std::ranges::find_if(
            successor_tasks,
            is_on_critical_path
        );

        if (next_it == successor_tasks.end()) {
            break;
        }

        curr_task_id = *next_it;
    }

    return critical_path;
}

cluster::node_id best_fitting_node(
    std::unordered_set<workflow::task_id> const & critical_path,
    workflow::workflow const & w,
    cluster::cluster const & c,
    bool const use_memory_requirements
) {
    if (!use_memory_requirements) {
        return c.best_performance_node();
    }

    // in our input model, the critical path is simply scheduled on
    // the best node with sufficient memory if we want to use the memory requirements
    auto critical_path_memories = critical_path 
        | std::views::transform([&w] (workflow::task_id const & t_id) {
            return w.get_task(t_id).memory_requirement;
        });
    double const critical_path_memory_requirement = *std::ranges::min_element(critical_path_memories);
    return c.best_performance_node(critical_path_memory_requirement);
}

// Critical path on processor

// Running time analysis:
// TODO

schedule::schedule cpop(
    cluster::cluster const & c, 
    workflow::workflow const & w,
    bool const use_memory_requirements
) {
    auto const downward_ranks = w.all_downward_ranks(
        c.mean_performance(),
        c.mean_bandwidth()
    );

    auto const upward_ranks = w.all_upward_ranks(
        c.mean_performance(),
        c.mean_bandwidth()
    );

    auto const task_priorities = compute_task_priorities(downward_ranks, upward_ranks);
    
    auto const critical_path = compute_critical_path(w, task_priorities);

    cluster::node_id const best_node = best_fitting_node(critical_path, w, c, use_memory_requirements);

    schedule::schedule s(c, use_memory_requirements);

    struct prioritized_task {
        // members can't be const because the priority queue needs this to be moveable
        workflow::task_id id;
        double priority;
        bool on_critical_path;
    };

    struct task_priority_compare {
        bool operator() (prioritized_task const & t0, prioritized_task const & t1) const {
            return t0.priority < t1.priority;
        }
    };

    std::priority_queue<prioritized_task, std::vector<prioritized_task>, task_priority_compare> prio_q;

    for (workflow::task_id const & t_id : w.get_independent_task_ids()) {
        prio_q.push({t_id, task_priorities.at(t_id), critical_path.contains(t_id)});
    }

    // copy incoming edges locally to modify to identify new independent tasks
    auto temp_incoming_edges = w.get_all_incoming_edges();
    
    while (!prio_q.empty()) {
        auto [curr_t_id, priority, on_critical_path] = prio_q.top();
        prio_q.pop();

        if (critical_path.contains(curr_t_id)) {
            s.insert_into_node_schedule(curr_t_id, best_node, w);
        } else {
            s.insert_into_best_eft_node_schedule(curr_t_id, w); 
        }

        for (auto const & [neighbor_id, weight] : w.get_task_outgoing_edges(curr_t_id)) {
            size_t const num_erased = temp_incoming_edges.at(neighbor_id).erase(curr_t_id);
            if (num_erased != 1) {
                throw std::runtime_error("Internal bug: incoming/outgoing edges are out of sync");
            }

            if (temp_incoming_edges.at(neighbor_id).empty()) {
                prio_q.push({
                    neighbor_id, 
                    task_priorities.at(neighbor_id), 
                    critical_path.contains(neighbor_id)
                });
            }
        }
    }

    return s;
}

} // namespace algorithms
