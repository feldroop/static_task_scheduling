#pragma once

#include <algorithm>
#include <functional>
#include <ranges>
#include <stdexcept>
#include <vector>

#include <schedule/time_interval.hpp>
#include <util/di_graph.hpp>

class workflow {
public:
    using task_id = size_t;
    struct task {
        task_id const id;
        double const compute_cost;
        size_t const memory_requirement;
    };

    using iterator = di_graph<task, double>::vertex_iterator;

private:
    di_graph<task, double> g;

public:
    // create a DAG workflow represetation based on the input specifications
    // it is assumed that the ids in from_ids and to_ids refer to the indices of the other arguments
    workflow(
        std::vector<double> const & computation_costs,
        std::vector<size_t> const & memory_requirements,
        std::vector<double> const & input_data_sizes,
        std::vector<double> const & output_data_sizes,
        std::vector<task_id> const & from_ids,
        std::vector<task_id> const & to_ids
    ) {
        if (
            computation_costs.size() != memory_requirements.size() ||
            computation_costs.size() != input_data_sizes.size() || 
            computation_costs.size() != output_data_sizes.size()
        ) {
            throw std::invalid_argument("Arguments for task parameters must have the same size.");
        } else if (from_ids.size() != to_ids.size()) {
            throw std::invalid_argument("Arguments for data exchange parameters must have the same size.");
        }

        for (size_t i = 0; i < computation_costs.size(); ++i) {
            if (computation_costs[i] == 0) {
                throw std::invalid_argument("All tasks need a computation cost of > 0");
            }

            task const t = task{i, computation_costs[i], memory_requirements[i]};
            g.add_vertex(t);
        }

        for (size_t i = 0; i < from_ids.size(); ++i) {
            if (output_data_sizes.at(from_ids.at(i)) != input_data_sizes.at(to_ids.at(i))) {
                throw std::invalid_argument("Input/Output data sizes for an edge don't match.");
            }

            bool const was_created = g.add_edge(
                from_ids.at(i), 
                to_ids.at(i), 
                output_data_sizes.at(from_ids.at(i))
            );

            if (!was_created) {
                throw std::invalid_argument("Task ids for data transfer endpoints are invalid.");
            }
        }
    }

    std::unordered_map<task_id, double> all_upward_ranks(
        double const mean_cluster_performance,
        double const mean_cluster_bandwidth
    ) const {
        std::unordered_map<task_id, double> upward_ranks{};
        
        auto topological_order = g.topological_order().value();
        for (task_id const t_id : std::views::reverse(topological_order)) {
            double const upward_rank = compute_upward_rank(
                upward_ranks, 
                mean_cluster_performance, 
                mean_cluster_bandwidth, 
                t_id
            );
            upward_ranks.insert({t_id, upward_rank});
        }

        return upward_ranks;
    }

    schedule::time_t get_sequential_makespan(double const best_cluster_node_performance) const {
        auto const & tasks = g.get_all_vertices();
        
        return std::transform_reduce(
            tasks.begin(),
            tasks.end(), 
            0.0,
            std::plus<>(),
            [best_cluster_node_performance] (auto const & t) {
                return t.compute_cost / best_cluster_node_performance;
            }
        );
    }

    std::unordered_map<task_id, double> const & get_task_incoming_edges(task_id const t_id) const {
        return g.get_incoming_edges(t_id);
    }

    std::unordered_map<task_id, double> const & get_task_outgoing_edges(task_id const t_id) const {
        return g.get_outgoing_edges(t_id);
    }

    task const & get_task(task_id const t_id) const {
        return g.get_vertex(t_id);
    }

    iterator begin() const {
        return g.get_all_vertices().begin();
    }

    iterator end() const {
        return g.get_all_vertices().end();
    }

    // number of tasks in the workflow
    size_t size() const {
        return g.get_all_vertices().size();
    }

private:
    double compute_upward_rank(
        std::unordered_map<task_id, double> const & upward_ranks,
        double const mean_cluster_performance,
        double const mean_cluster_bandwidth,
        task_id const t_id
    ) const {
        double upward_rank = g.get_vertex(t_id).compute_cost * mean_cluster_performance;

        auto outgoing_ranks = g.get_outgoing_edges(t_id) 
            | std::views::transform([&upward_ranks, mean_cluster_bandwidth] (auto const & edge) {
                auto const & [neighbor_id, data_transfer] = edge;
                return data_transfer / mean_cluster_bandwidth + upward_ranks.at(neighbor_id);
            }
        );

        auto const max_it = std::max_element(outgoing_ranks.begin(), outgoing_ranks.end());
        if (max_it != outgoing_ranks.end()) {
            upward_rank += *max_it;
        }

        return upward_rank;
    }
};
