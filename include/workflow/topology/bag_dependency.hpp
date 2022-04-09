#pragma once

#include <stdexcept>
#include <unordered_map>

#include <workflow/task_bag.hpp>
#include <workflow/topology/topology.hpp>

namespace workflow::topology {

// describes a collective dependency between two tasks bags, source and target
enum class bag_dependency {
    // source and target bag have the same number of tasks and the i-th task
    // of the source bag depends on the i-th task of the target bag
    one_to_one,
    
    // there are more tasks in the target bag than in the source bag and
    // the tasks from the target bag are divided as evenly as possible onto the tasks 
    // of the source bag such that each task in the target bag depends on a single
    // task in the source bag
    distribute,

    // there are less tasks in the target bag than in the source bag and
    // the tasks from the source bag are divided as evenly as possible onto the tasks 
    // of the target bag such that each task in the source bag gives a dependency onto
    // a single task in the target bag
    aggregate,

    // a non-trivial dependency pattern that is currently not supported
    complex
};

// dependency_pattern[source_id][target_id] -> bag dependency between source and target
using dependency_pattern = std::unordered_map<task_bag_id, 
                            std::unordered_map<task_bag_id, bag_dependency>>;

dependency_pattern to_dependency_pattern(topology const top) {
    switch (top) {
        case topology::epigenome:
            return {
                {0, {{1, bag_dependency::distribute}}},
                {1, {{2, bag_dependency::one_to_one}}},
                {2, {{3, bag_dependency::one_to_one}}},
                {3, {{4, bag_dependency::one_to_one}}},
                {4, {{5, bag_dependency::aggregate}}},
                {5, {{6, bag_dependency::aggregate}}}, // could also be one_to_one from the paper
                {6, {{7, bag_dependency::one_to_one}}}
            };

        case topology::cybershake:
            // this is different than in the actual CyberShake workflow, because we
            // can't model it using the task bags
            return {
                {0, {{1, bag_dependency::distribute}}},
                {1, {{3, bag_dependency::one_to_one}}},
                {2, {{3, bag_dependency::distribute}}},
                {3, {{4, bag_dependency::aggregate}}}
            };

        case topology::ligo:
            return {
                {0, {{1, bag_dependency::one_to_one}}},
                {1, {{2, bag_dependency::aggregate}}},
                {2, {{3, bag_dependency::distribute}}},
                {3, {{4, bag_dependency::one_to_one}}},
                {4, {{5, bag_dependency::aggregate}}}
            };

        case topology::montage:
            return {
                {0, {{1, bag_dependency::complex}, {4, bag_dependency::one_to_one}}},
                {1, {{2, bag_dependency::aggregate}}},
                {2, {{3, bag_dependency::one_to_one}}},
                {3, {{4, bag_dependency::distribute}}},
                {4, {{5, bag_dependency::aggregate}}},
                {5, {{6, bag_dependency::one_to_one}}},
                {6, {{7, bag_dependency::one_to_one}}},
                {7, {{8, bag_dependency::one_to_one}}}
            };

        case topology::none:
            throw std::runtime_error("In this mode, a topology must be supplied on the comand line.");

        default:
            throw std::runtime_error("No dependency pattern is known for this topology.");
    }
}

} // namespace workflow::topology
