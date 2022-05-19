#pragma once

#include <iostream>
#include <ranges>
#include <vector>

#include <cluster/cluster.hpp>
#include <schedule/schedule.hpp>
#include <workflow/task.hpp>

namespace algorithms {

struct task_group {
    using iterator = std::vector<workflow::task_id>::iterator;

    std::vector<workflow::task_id> task_ids{};
    size_t cardinality{0};
    double workload{0.0};

    void add_task(workflow::task const & task) {
        task_ids.push_back(task.id);
        ++cardinality;
        workload += task.workload;
    }

    iterator begin() {
        return task_ids.begin();
    }

    iterator end() {
        return task_ids.begin();
    }
};

// return sequence of <groups> many numbers that add up to <total> 
// split as evenly as possible (numbers differ by at most one)
std::vector<size_t> split_most_evenly(size_t const total, size_t const num_groups) {
    std::vector<size_t> group_sizes(num_groups);
    
    size_t const ratio = total / num_groups;
    size_t remainder = total % num_groups;
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
