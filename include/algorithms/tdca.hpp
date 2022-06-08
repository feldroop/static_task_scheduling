#pragma once

#include <numeric>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <algorithms/common_clustering_based.hpp>
#include <cluster/cluster.hpp>
#include <io/command_line_arguments.hpp>
#include <io/issue_warning.hpp>
#include <schedule/schedule.hpp>
#include <util/timepoint.hpp>
#include <workflow/data_transfer_cost.hpp>
#include <workflow/node_task_matrix.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {

std::vector<workflow::task_id> task_ids_sorted_by_level_ascending(
    workflow::workflow const & w,
    std::unordered_map<workflow::task_id, double> const & level
) {
    auto task_ids = std::ranges::iota_view{0ul, w.size()};
    std::vector<workflow::task_id> level_task_ids(task_ids.begin(), task_ids.end());

    std::ranges::sort(level_task_ids, {}, [&level] (auto const & t_id) {
        return level.at(t_id);
    });

    return level_task_ids;
}

// pop and returns last element of the vector
// assumes that v.size() >= 1
template<typename T>
T pop_back_and_return(std::vector<T> & v) {
    T item = v.back();
    v.pop_back();
    return item;
}

std::optional<workflow::task_id> find_better_predecessor(// the predecessor is k in the paper
    cluster::cluster const & c,
    workflow::workflow const & w,
    node_task_matrix<util::timepoint> const & eft,
    std::unordered_set<workflow::task_id> const & assigned_task_ids,
    workflow::task_id const & curr_task_id,
    cluster::node_id const & curr_node_id
) {
    cluster::node_id const best_node_id = c.best_performance_node();

    auto candidate_predecessors = w.get_task_incoming_edges(curr_task_id) 
        | std::views::filter([&] (auto const & edge) {
            auto const & [neighbor_id, data_transfer] = edge;

            auto const data_transfer_cost = workflow::get_raw_data_transfer_cost(
                data_transfer, 
                c.uniform_bandwidth()
            );

            return !assigned_task_ids.contains(neighbor_id)
                && eft(neighbor_id, curr_node_id) <= eft(neighbor_id, best_node_id) + data_transfer_cost;
        });

    auto const best_it = std::ranges::min_element(candidate_predecessors, {}, [&] (auto const & edge) {
        auto const & [neighbor_id, data_transfer] = edge;
        return eft(neighbor_id, curr_node_id);
    });

    if (best_it != candidate_predecessors.end()) {
        return best_it->first;
    }

    return std::nullopt;
}

schedule::schedule schedule_from_groups(
    cluster::cluster const & c,
    workflow::workflow const & w,
    std::vector<task_group> const & groups,
    bool const unscheduled_predecessors_allowed = false,
    bool const use_memory_requirements = false
) {
    schedule::schedule s(c, use_memory_requirements);

    std::unordered_map<workflow::task_id, std::vector<cluster::node_id>> task_to_nodes;

    for (auto const & t : w) {
        task_to_nodes.insert({t.id, {}});
    }

    for (cluster::node_id const n_id : std::ranges::iota_view{0ul, c.size()}) {
        auto const task_ids = groups.at(n_id).get_tasks_in_topologcal_order(w);
        for (workflow::task_id const & t_id : task_ids) {
            task_to_nodes.at(t_id).push_back(n_id);
        }
    }

    for (workflow::task_id const & t_id : w.get_task_topological_order()) {
        for (cluster::node_id const n_id : task_to_nodes.at(t_id)) {
            s.insert_into_node_schedule(t_id, n_id, w, unscheduled_predecessors_allowed);
        }
    }

    return s;
}

std::vector<task_group> groups_from_schedule(
    cluster::cluster const & c,
    workflow::workflow const & w,
    schedule::schedule const & s
) {
    std::vector<task_group> groups(c.size());

    for (cluster::node_id const curr_node_id : std::ranges::iota_view{0ul, c.size()}) {
        auto const task_ids = s.get_tasks_of_node(curr_node_id);
        for (workflow::task_id const & t_id : task_ids) {
            auto const & t = w.get_task(t_id);
            groups.at(curr_node_id).add_task(t);
        }
    }

    return groups;
}

// in the paper: Initial task clustering
std::vector<task_group> initial_groups(
    cluster::cluster const & c,
    workflow::workflow const & w,
    std::unordered_map<workflow::task_id, double> const & level,
    std::vector<workflow::task_id> const & cpred,
    node_task_matrix<util::timepoint> const & eft
) {
    std::vector<task_group> groups(c.size());

    // lowest "level" score to the back
    auto const sorted_task_ids = task_ids_sorted_by_level_ascending(w, level);
    // best node ids to the back
    auto remaining_node_ids = c.node_ids_sorted_by_performance_ascending();

    // keeps track of tasks that were assigned in the while bubbling up from the
    // current task in the main loop
    std::unordered_set<workflow::task_id> assigned_task_ids;

    auto const & independent_task_ids = w.get_independent_task_ids();
    cluster::node_id const best_node_id = c.best_performance_node();

    for (workflow::task_id curr_task_id /*i in the paper*/ : sorted_task_ids) {
        if (assigned_task_ids.contains(curr_task_id)) {
            continue;
        }

        if (remaining_node_ids.empty()) {
            break;
        }
        auto const curr_node_id = pop_back_and_return(remaining_node_ids);

        auto const & curr_task = w.get_task(curr_task_id);
        groups.at(curr_node_id).add_task(curr_task);
        assigned_task_ids.insert(curr_task_id);
        
        // "bubble" up and fill cluster from the current task using cpred
        while (!independent_task_ids.contains(curr_task_id)) {
            auto next_task_id = cpred.at(curr_task_id); // j in the paper
            
            auto const & curr_task_incoming_edges = w.get_task_incoming_edges(curr_task_id);
            auto const data_transfer_cost = workflow::get_raw_data_transfer_cost(
                curr_task_incoming_edges.at(next_task_id), 
                c.uniform_bandwidth()
            );

            if (
                curr_task_incoming_edges.size() > 1 
                && (
                    // I am wondering wether it would make more sense to always check
                    // for whether next_task_id is assigned. But this is what it says in the paper
                    assigned_task_ids.contains(next_task_id)
                    || eft(next_task_id, curr_node_id) > eft(next_task_id, best_node_id) 
                        + data_transfer_cost
                )
            ) {
                auto const better_task_opt = find_better_predecessor(
                    c,
                    w, 
                    eft,
                    assigned_task_ids,
                    curr_task_id,
                    curr_node_id
                );

                if (better_task_opt) {
                    next_task_id = better_task_opt.value();
                } else {
                    break;
                }
            }

            auto const & next_task = w.get_task(next_task_id);
            groups.at(curr_node_id).add_task(next_task);
            assigned_task_ids.insert(next_task_id);

            curr_task_id = next_task_id;
        }
    }

    auto s = schedule_from_groups(c, w, groups, true);
    if (assigned_task_ids.size() < w.size()) {
        // add remaing tasks to the respective groups that minimize their est
        for (workflow::task_id const curr_task_id : w.get_task_topological_order()) {
            if (assigned_task_ids.contains(curr_task_id)) {
                continue;
            }

            // in the paper it says minimize the starting time, so we use the est and the eft
            cluster::node_id const n_id = s.insert_into_best_eft_node_schedule(curr_task_id, w, true);
            
            auto const & t = w.get_task(curr_task_id);
            groups.at(n_id).add_task(t);
        }
    }

    return groups;
}

void task_duplication(
    cluster::cluster const & c,
    workflow::workflow const & w,
    std::vector<task_group> & groups,
    std::vector<workflow::task_id> const & cpred,
    size_t const num_iterations = 4,
    [[maybe_unused]] bool const use_memory_requirements = false
) {
    using std::ranges::iota_view;

    auto const sorted_node_ids = c.node_ids_sorted_by_performance_ascending();
    auto unoccupied_nodes_view = sorted_node_ids
        | std::views::filter([&groups] (cluster::node_id const & n_id) {
            return groups.at(n_id).empty();
        });

    std::vector<cluster::node_id> unoccupied_nodes(
        unoccupied_nodes_view.begin(),
        unoccupied_nodes_view.end()
    );

    schedule::schedule curr_sched = schedule_from_groups(c, w, groups);

    for ([[maybe_unused]] auto const iter_num : iota_view{0ul, num_iterations}) {
        for (cluster::node_id const curr_node_id : iota_view{0ul, c.size()}) {
            auto task_ids = groups.at(curr_node_id).get_tasks_in_topologcal_order(w);

            if (task_ids.size() > 1) {
                for (size_t const i : iota_view{1ul, task_ids.size()} | std::views::reverse) {
                    cluster::node_id const next_node_id = unoccupied_nodes.empty()
                        ? c.best_performance_node()
                        : pop_back_and_return(unoccupied_nodes);
                    
                    if (task_ids.at(i - 1) != cpred.at(task_ids.at(i))) {
                        std::vector<task_group> temp_groups = groups;
                        
                        // move all the tasks before the i-th to the new node
                        std::unordered_set<workflow::task_id> move_ids(task_ids.begin(), task_ids.begin() + i);
                        temp_groups.at(curr_node_id).remove_tasks(w, move_ids);

                        for (workflow::task_id const & move_id : move_ids) {
                            temp_groups.at(next_node_id).add_task_id(w, move_id);
                        }

                        // add predecessor trail of i to the current node
                        auto curr_task_id = task_ids.at(i);
                        while (!w.get_independent_task_ids().contains(curr_task_id)) {
                            curr_task_id = cpred.at(curr_task_id);
                            temp_groups.at(curr_node_id).add_task_id(w, curr_task_id);
                        }

                        // check whether these moves improved the makespan
                        schedule::schedule const temp_sched = schedule_from_groups(c, w, temp_groups);

                        if (temp_sched.get_makespan() <= curr_sched.get_makespan()) {
                            curr_sched = std::move(temp_sched);
                            groups = std::move(temp_groups);
                            break;
                        }
                    }
                }
            }

            // recompute task_ids, because they might have changed
            task_ids = groups.at(curr_node_id).get_tasks_in_topologcal_order(w);

            if (
                !task_ids.empty()
                && !w.get_independent_task_ids().contains(task_ids.front())
            ) {
                std::vector<task_group> temp_groups = groups;

                // add predecessor trail of first task to the current node
                auto curr_task_id = task_ids.front();
                while (!w.get_independent_task_ids().contains(curr_task_id)) {
                    curr_task_id = cpred.at(curr_task_id);
                    temp_groups.at(curr_node_id).add_task_id(w, curr_task_id);
                }

                // check whether these additions improved the makespan
                schedule::schedule const temp_sched = schedule_from_groups(c, w, temp_groups);

                if (temp_sched.get_makespan() <= curr_sched.get_makespan()) {
                    curr_sched = std::move(temp_sched);
                    groups = std::move(temp_groups);
                }
            }
        }
    }
}

void merge_nodes(
    cluster::cluster const & c,
    workflow::workflow const & w,
    std::vector<task_group> & groups,
    size_t const num_iterations = 4,
    [[maybe_unused]] bool const use_memory_requirements = false
) {
    using std::ranges::iota_view;

    schedule::schedule curr_sched = schedule_from_groups(c, w, groups);

    cluster::node_id const best_node_id = c.best_performance_node();

    for ([[maybe_unused]] auto const iter_num : iota_view{0ul, num_iterations}) {
        for (cluster::node_id const curr_node_id : iota_view{0ul, c.size()}) {
            std::vector<task_group> temp_groups = groups;

            // move all the tasks before the i-th to the best node
            std::unordered_set<workflow::task_id> move_ids = temp_groups.at(curr_node_id)
                .clear_and_return_task_ids();

            for (workflow::task_id const & move_id : move_ids) {
                temp_groups.at(best_node_id).add_task_id(w, move_id);
            }

            // check whether these moves improved the makespan
            schedule::schedule const temp_sched = schedule_from_groups(c, w, temp_groups);

            if (temp_sched.get_makespan() <= curr_sched.get_makespan()) {
                curr_sched = std::move(temp_sched);
                groups = std::move(temp_groups);
            }
        }
    }
}

void refine_edges(
    cluster::cluster const & c,
    workflow::workflow const & w,
    std::vector<task_group> & groups,
    [[maybe_unused]] bool const use_memory_requirements = false
) {
    schedule::schedule curr_sched = schedule_from_groups(c, w, groups);
    auto const differing_edges = curr_sched.get_different_node_edges(w);

    for (auto const & edge : differing_edges) {
        std::vector<task_group> temp_groups = groups;

        // add the "from" task to the "to" node
        temp_groups.at(edge.to_n_id).add_task_id(w, edge.from_t_id);
        
        // remove "from" task from "from" node if it has no outgoing edges on the node
        bool outgoing_edges_in_group = false;
        for (auto const [succ_t_id, data_transfer] : w.get_task_outgoing_edges(edge.from_t_id)) {
            if (temp_groups.at(edge.from_n_id).contains(succ_t_id)) {
                outgoing_edges_in_group = true;
            }
        }

        if (
            !outgoing_edges_in_group
            // the "from" task might have been deleted already
            && !temp_groups.at(edge.from_n_id).contains(edge.from_t_id) 
        ) {
            std::unordered_set<workflow::task_id> move_ids{edge.from_t_id};
            temp_groups.at(edge.from_n_id).remove_tasks(w, move_ids);
        }

        // check whether this move improved the makespan
        schedule::schedule const temp_sched = schedule_from_groups(c, w, temp_groups);

        if (temp_sched.get_makespan() <= curr_sched.get_makespan()) {
            curr_sched = std::move(temp_sched);
            groups = std::move(temp_groups);
        }
    }
}

schedule::schedule tdca(
    cluster::cluster const & c, 
    workflow::workflow const & w,
    io::command_line_arguments const & args
) {
    if (args.use_memory_requirements) {
        io::issue_warning(args, "Memory requirements not implemented/used for RBCA");
    }

    auto const [est, eft, cpred] = w.compute_est_and_eft(c); // eft == ect in the paper

    // in our model, the favorite nodes of all tasks are simply 
    // the ones with the best performance
    auto const favorite_nodes = c.node_ids_sorted_by_performance_descending();

    // borrow code from the HEFT implementation, hence the name upward ranks
    auto const level = w.all_upward_ranks(
        c.worst_performance_node(),
        c.uniform_bandwidth()
    );

    auto groups = initial_groups(c, w, level, cpred, eft);

    task_duplication(c, w, groups, cpred);

    merge_nodes(c, w, groups);

    refine_edges(c, w, groups);

    return schedule_from_groups(c, w, groups);
}

} // namespace algorithms
