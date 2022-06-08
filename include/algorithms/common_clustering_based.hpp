#pragma once

#include <iostream>
#include <ranges>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include <cluster/cluster.hpp>
#include <schedule/schedule.hpp>
#include <workflow/task.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {

struct task_group {
    using iterator = std::unordered_set<workflow::task_id>::iterator;

    std::unordered_set<workflow::task_id> task_ids{};
    size_t cardinality{0};
    double workload{0.0};

    void add_task(workflow::task const & task) {
        task_ids.insert(task.id);
        ++cardinality;
        workload += task.workload;
    }

    void add_task_id(workflow::workflow const & w, workflow::task_id const t_id) {
        auto const & t = w.get_task(t_id);
        add_task(t);
    }

    // assumes that move ids are in the groups, otherwise errors
    void remove_tasks(
        workflow::workflow const & w, 
        std::unordered_set<workflow::task_id> const & move_ids
    ) {
        cardinality -= move_ids.size();

        for (workflow::task_id const & move_id : move_ids) {
            workload -= w.get_task(move_id).workload;
        }

        auto erased = std::erase_if(task_ids, [&move_ids] (workflow::task_id const & task_id) {
            return move_ids.contains(task_id);
        });

        if (erased != move_ids.size()) {
            throw std::runtime_error("Internal bug: task_group does not contain all move ids.");
        }
    }

    iterator begin() {
        return task_ids.begin();
    }

    iterator end() {
        return task_ids.begin();
    }

    bool empty() const {
        return task_ids.empty();
    }

    std::unordered_set<workflow::task_id> clear_and_return_task_ids() {
        std::unordered_set<workflow::task_id> temp = std::move(task_ids);
        task_ids = std::unordered_set<workflow::task_id>();
        cardinality = 0;
        workload = 0.0;

        return temp;
    }

    std::vector<workflow::task_id> get_tasks_in_topologcal_order(workflow::workflow const & w) const {
        std::vector<workflow::task_id> ordered_task_ids(task_ids.begin(), task_ids.end());
        std::ranges::sort(ordered_task_ids, {}, [&w] (workflow::task_id const & t_id) {
            return w.topological_task_rank(t_id);
        });
        
        return ordered_task_ids;
    }

    bool contains(workflow::task_id const t_id) const {
        return task_ids.contains(t_id);
    }
};

// return sequence of <groups> many numbers that add up to <total> 
// split as evenly as possible (numbers differ by at most one)
std::vector<size_t> split_most_evenly(size_t const total, size_t const num_groups) {
    std::vector<size_t> group_sizes(num_groups);
    
    size_t const ratio = total / num_groups;
    size_t const remainder = total % num_groups;
    // <remainder> many tasks cannot be evenly disrtibuted using the ratio <ratio>
    // hence the first <remainder> many task groups get one more task

    std::fill(group_sizes.begin(), group_sizes.begin() + remainder, ratio + 1);
    std::fill(group_sizes.begin() + remainder, group_sizes.end(), ratio);

    return group_sizes;
}

// match most expensive group to best cluster, and so on
void select_good_processors_for_expensive_groups(
    cluster::cluster const & c, 
    workflow::workflow const & w,
    schedule::schedule & s,
    std::vector<task_group> & groups,
    [[maybe_unused]] bool const use_memory_requirements
) {
    auto const node_ids = c.node_ids_sorted_by_performance_descending();
    
    size_t curr_node_index{0};
    
    std::ranges::sort(groups, std::ranges::greater(), [] (task_group const & group) {
        return group.workload;
    });

    for (task_group const & group : groups) {
        for (workflow::task_id const t_id : group.task_ids) {
            s.insert_into_node_schedule(t_id, node_ids.at(curr_node_index), w);
        }
        ++curr_node_index;
    }
}

} // namespace algorithms
