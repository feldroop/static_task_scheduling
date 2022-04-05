#pragma once

#include <numeric>
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

// index of the returned vector is to task bag id to which the ids at that index belong
std::vector<std::vector<task_id>> expand_task_bags_into_ids(std::vector<task_bag> const & bags) {
    std::vector<std::vector<task_id>> ids(bags.size());

    task_id first_id{0};

    for (task_bag const & bag : bags) {
        std::vector<task_id> & curr_ids = ids.at(bag.id);
        curr_ids.resize(bag.cardinality);
        std::iota(curr_ids.begin(), curr_ids.end(), first_id);

        first_id += bag.cardinality;
    }

    return ids;
}

} // namespace workflow
