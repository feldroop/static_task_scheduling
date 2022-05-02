#pragma once

#include <cmath>
#include <iterator>
#include <ranges>
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include <algorithms/common_clustering_based.hpp>
#include <cluster/cluster.hpp>
#include <io/command_line_arguments.hpp>
#include <io/issue_warning.hpp>
#include <schedule/schedule.hpp>
#include <workflow/task.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {

struct dependency_correlation_matrix {
private:
    std::vector<std::vector<double>> data;
    std::unordered_map<workflow::task_id, size_t> task_id_to_index;

public:
    dependency_correlation_matrix(
        workflow::workflow const & w,
        std::vector<workflow::task_id> const & bag
    ) : data(bag.size()) {
        if (data.empty()) {
            return;
        }

        std::vector<std::vector<workflow::task_id>> successors{};

        for (workflow::task_id const t_id : bag) {
            auto successor_view = w.get_task_outgoing_edges(t_id)
                | std::views::transform([] (auto const & edge) {
                    auto const & [neighbor_id, weight] = edge;
                    return neighbor_id;
                });
            
            successors.emplace_back(successor_view.begin(), successor_view.end());
            std::ranges::sort(successors.back());
        }
        
        for (size_t i = 0; i < bag.size() - 1; ++i) {
            double const num_i_successors = successors.at(i).size();

            for (size_t j = i + 1; j < bag.size(); ++j) {
                double const num_j_successors = successors.at(j).size();
                
                std::vector<workflow::task_id> i_j_intersection{};

                std::ranges::set_intersection(
                    successors.at(i), 
                    successors.at(j), 
                    std::back_inserter(i_j_intersection)
                );

                double const num_i_j_successors = i_j_intersection.size();
                double const i_j_cor = num_i_j_successors / 
                    std::sqrt(num_i_successors * num_j_successors);

                data.at(i).push_back(i_j_cor);
            }
        }

        for (size_t i = 0; i < bag.size(); ++i) {
            task_id_to_index.insert({bag.at(i), i});
        }
    }

    double get(workflow::task_id const t0_id, workflow::task_id const t1_id) const {
        size_t const t0_index = task_id_to_index.at(t0_id);
        size_t const t1_index = task_id_to_index.at(t1_id);

        auto const [i, j] = std::minmax(t0_index, t1_index);

        return data.at(i).at(j - i - 1);
    }
};

// workflow::task_id find_most_correlated_task(
//     workflow::task_id const curr_t_id,
//     std::set<workflow::task_id> const & remaining_task_ids,
//     dependency_correlation_matrix const & cor
// ) {
//     // TODO
//     return 0;
// }

std::vector<task_group> dependency_balanced_task_groups(
    workflow::workflow const & w,
    std::vector<workflow::task_id> const & bag, 
    size_t const num_cluster_nodes
) {
    size_t const num_groups = std::min(bag.size(), num_cluster_nodes);
    std::vector<task_group> groups(num_groups);

    dependency_correlation_matrix const cor(w, bag);

    auto const group_sizes = split_most_evenly(bag.size(), num_groups);
    std::set<workflow::task_id> remaining_task_ids(bag.begin(), bag.end());

    for (size_t i = 0; i < num_groups; ++i) {
        size_t const group_size = group_sizes.at(i);
        task_group & group = groups.at(i);

        if (group_size == 1) {
            if (remaining_task_ids.empty()) {
                throw std::runtime_error("Internal bug: DBCA task list empty too early.");
            }

            workflow::task_id const t_id = *remaining_task_ids.begin();
            group.add_task(w.get_task(t_id));

            remaining_task_ids.erase(t_id);
            continue;
        }

        // TODO
    }
    

    // TODO
    return {};
}

// Dependency balance clustering algorithm

// Running time analysis:
// TODO

schedule::schedule dbca(
    cluster::cluster const & c, 
    workflow::workflow const & w,
    io::command_line_arguments const & args
) {
    schedule::schedule s(c, args.use_memory_requirements);

    if (args.use_memory_requirements) {
        io::issue_warning(args, "Memory requirements not implemented/used for DBCA");
    }

    io::issue_warning(args, "DBCA not implemented yet");

    // we use our bags instead of the levels as defined in the original paper
    // (makes sense, but is not always equal)
    auto const & task_ids_per_bag = w.get_task_ids_per_bag();

    for (auto const & bag: task_ids_per_bag) {
        auto groups = dependency_balanced_task_groups(w, bag, c.size());
        select_good_processors_for_expensive_groups(
            c, w, s, groups, args.use_memory_requirements
        );
    }

    return s;
}

}  // namespace algorithms
