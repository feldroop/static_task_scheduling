#pragma once

#include <algorithm>
#include <unordered_set>
#include <vector>

#include <workflow/task.hpp>
#include <workflow/task_dependency.hpp>
#include <workflow/task_bag.hpp>

namespace workflow::topology {

void remove_bag_dependencies(
    std::vector<task_dependency> & task_dependencies,
    task_bag_id const source_bag_id,
    task_bag_id const target_bag_id,
    std::vector<task_bag> const & bags
) {
    auto const task_ids_per_bag = expand_task_bags_into_ids(bags);

    auto const source_bag_task_ids = task_ids_per_bag.at(source_bag_id);
    auto const target_bag_task_ids = task_ids_per_bag.at(target_bag_id);

    std::unordered_set<task_id> const source_bag_task_id_set(
        source_bag_task_ids.begin(), 
        source_bag_task_ids.end()
    );

    std::unordered_set<task_id> const target_bag_task_id_set(
        target_bag_task_ids.begin(), 
        target_bag_task_ids.end()
    );

    auto const is_between_source_and_target_bag = [&source_bag_task_id_set, &target_bag_task_id_set] 
        (task_dependency const & dep) {
        return source_bag_task_id_set.contains(dep.from_id)
            && target_bag_task_id_set.contains(dep.to_id);
    };

    auto const [first, last] = std::ranges::remove_if(task_dependencies, is_between_source_and_target_bag);
    task_dependencies.erase(first, last);
}

} // namespace workflow::topology
