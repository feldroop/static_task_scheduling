#pragma once

#include <tuple>
#include <vector>

#include <workflow/task.hpp>
#include <workflow/task_bag.hpp>

namespace workflow {

using unpacked_task_bags = std::tuple<std::vector<task>, std::vector<double>, std::vector<double>>;

unpacked_task_bags expand_task_bags(std::vector<task_bag> const & bags) {
    std::vector<task> tasks;
    std::vector<double> input_data_sizes;
    std::vector<double> output_data_sizes;

    size_t id{0};

    for (task_bag const & bag : bags) {
        for (size_t count = 0; count < bag.cardinality; ++count) {
            tasks.emplace_back(id, bag.workload, bag.memory_requirement);
            input_data_sizes.push_back(bag.input_data_size);
            output_data_sizes.push_back(bag.output_data_size);

            ++id;
        }
    }

    return std::make_tuple(std::move(tasks), std::move(input_data_sizes), std::move(output_data_sizes));
}

} // namespace workflow
