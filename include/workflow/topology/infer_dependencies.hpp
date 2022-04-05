#pragma once

#include <vector>

#include <workflow/expand_task_bags.hpp>
#include <workflow/task.hpp>
#include <workflow/task_dependency.hpp>
#include <workflow/task_bag.hpp>
#include <workflow/topology/bag_dependency.hpp>
#include <workflow/topology/topology.hpp>

namespace workflow::topology {

void expand_bag_dependency(
    bag_dependency bag_dep,
    std::vector<task_dependency> & task_dependencies,
    std::vector<task_id> const & source_bag_task_ids,
    std::vector<task_id> const & target_bag_task_ids
) {
    size_t const source_cardinality = source_bag_task_ids.size();
    size_t const target_cardinality = target_bag_task_ids.size();

    switch (bag_dep) {
        case bag_dependency::one_to_one: {
            if (source_cardinality != target_cardinality) {
                throw std::runtime_error("Bags with one-to-one dependency must have equal cardinaliy.");
            }

            for (size_t i = 0; i < source_cardinality; ++i) {
                task_dependencies.emplace_back(source_bag_task_ids.at(i), target_bag_task_ids.at(i));
            }
        }
        break;

        case bag_dependency::distribute: {
            if (source_cardinality > target_cardinality) {
                throw std::runtime_error(
                    "Source bags with a distribute dependency must have a smaller or "
                    "equal cardinality in comparison to their target bag."
                );
            }

            size_t const ratio = target_cardinality / source_cardinality;
            size_t remainder = target_cardinality % source_cardinality;
            // <remainder> many tasks cannot be evenly disrtibuted using the ratio <ratio>
            // hence the first <remainder> many source bag tasks get one more outgoing dependency

            size_t first_target_task_id{0};

            for (task_id const & source_task_id : source_bag_task_ids) {
                size_t const num_outgoing = (remainder == 0) ? ratio : ratio + 1;

                for (size_t i = 0; i < num_outgoing; ++i) {
                    task_id const & target_task_id = target_bag_task_ids.at(first_target_task_id + i);
                    task_dependencies.emplace_back(source_task_id, target_task_id);
                }

                first_target_task_id += num_outgoing;
                if (remainder != 0) {
                    --remainder;
                }
            }
        }
        break;

        case bag_dependency::aggregate: {
            if (source_cardinality < target_cardinality) {
                throw std::runtime_error(
                    "Source bags with an aggregate cardinality must have a larger or "
                    "equal cardinality in comparison to their target bag."
                );
            }

            size_t const ratio = source_cardinality / target_cardinality;
            size_t remainder = source_cardinality % target_cardinality;
            // <remainder> many tasks cannot be evenly distributed using the ratio <ratio>
            // hence the first <remainder> many target bag tasks get one more incoming dependency

            size_t first_source_task_id{0};

            for (task_id const & target_task_id : target_bag_task_ids) {
                size_t const num_incoming = (remainder == 0) ? ratio : ratio + 1;

                for (size_t i = 0; i < num_incoming; ++i) {
                    task_id const & source_task_id = source_bag_task_ids.at(first_source_task_id + i);
                    task_dependencies.emplace_back(source_task_id, target_task_id);
                }

                first_source_task_id += num_incoming;
                if (remainder != 0) {
                    --remainder;
                }
            }
        }
        break;

        case bag_dependency::complex: {
            throw std::runtime_error(
                "The desired workflow architecture contains a complex bag "
                "dependency that is not supported."
            );
        }
        break;

        default:
            throw std::runtime_error("Unknown dependency between bags cannot be handled.");
    }
}

std::vector<task_dependency> infer_dependencies(topology const top, std::vector<task_bag> const & bags) {
    dependency_pattern dep_pattern = to_dependency_pattern(top);

    std::vector<task_dependency> task_dependencies{};
    std::vector<std::vector<task_id>> task_ids_per_bag = expand_task_bags_into_ids(bags);

    for (task_bag const & source_bag : bags) {
        if (!dep_pattern.contains(source_bag.id)) {
            continue;
        }

        for (auto const & [target_bag_id, bag_dep] : dep_pattern.at(source_bag.id)) {
            expand_bag_dependency(
                bag_dep,
                task_dependencies,
                task_ids_per_bag.at(source_bag.id), 
                task_ids_per_bag.at(target_bag_id)
            );
        }
    }

    return task_dependencies;
}

} // namespace workflow::topology
