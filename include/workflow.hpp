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

    enum class task_order {
        upward_rank
    };

private:
    di_graph<task, double> g;

public:
    // create a DAG workflow represetation based on the input specifications
    workflow(
        std::vector<double> const & computation_costs,
        std::vector<size_t> const & memory_requirements,
        std::vector<size_t> const & from_ids,
        std::vector<size_t> const & to_ids,
        std::vector<double> const & weights
    ) {
        if (computation_costs.size() != memory_requirements.size()) {
            throw std::invalid_argument("Arguments for task parameters must have the same size.");
        } else if (
            from_ids.size() != to_ids.size() ||
            to_ids.size() != weights.size()
        ) {
            throw std::invalid_argument("Arguments for data exchange parameters must have the same size.");
        }

        std::vector<task_id> ids{};

        for (size_t i = 0; i < computation_costs.size(); ++i) {
            if (computation_costs[i] == 0) {
                throw std::invalid_argument("All tasks need a computation cost of > 0");
            }

            task const t = task{i, computation_costs[i], memory_requirements[i]};
            task_id const id = g.add_vertex(t);
            ids.push_back(id);
        }

        for (size_t i = 0; i < from_ids.size(); ++i) {
            bool const was_created = g.add_edge(
                ids.at(from_ids.at(i)), 
                ids.at(to_ids.at(i)), 
                ids.at(weights.at(i))
            );

            if (!was_created) {
                throw std::invalid_argument("Task ids for data transfer endpoints are invalid.");
            }
        }
    }

    std::vector<task_id> task_ids_sorted_by(
        task_order const order,
        double const mean_cluster_performance
    ) const {
        std::vector<task_id> priority_list(g.get_all_vertices().size());
        std::iota(priority_list.begin(), priority_list.end(), 0);

        std::unordered_map<task_id, double> ranks;
        switch (order) {
            case task_order::upward_rank : ranks = compute_all_upward_ranks(mean_cluster_performance);
            break;
        }

        std::ranges::sort(priority_list, 
            [&ranks] (workflow::task_id const & t_id0, workflow::task_id const & t_id1) {
                return ranks.at(t_id0) > ranks.at(t_id1);
            }
        );

        return priority_list;
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

private:
    std::unordered_map<task_id, double> compute_all_upward_ranks(
        double const mean_cluster_performance
    ) const {
        std::unordered_map<task_id, double> upward_ranks{};
        
        auto topological_order = g.topological_order().value();
        for (task_id const t_id : std::views::reverse(topological_order))
        {
            upward_ranks.insert({t_id, compute_upward_rank(upward_ranks, mean_cluster_performance, t_id)});
        }

        return upward_ranks;
    }

    double compute_upward_rank(
        std::unordered_map<task_id, double> const & upward_ranks,
        double const mean_cluster_performance,
        task_id const t_id
    ) const {
        double upward_rank = g.get_vertex(t_id).compute_cost * mean_cluster_performance;

        auto outgoing_ranks = g.get_outgoing_edges(t_id) 
            | std::views::transform([&upward_ranks] (auto const & edge) {
                auto const & [neighbor_id, weight] = edge;
                return weight + upward_ranks.at(neighbor_id);
            }
        );

        auto const max_it = std::max_element(outgoing_ranks.begin(), outgoing_ranks.end());
        if (max_it != outgoing_ranks.end()) {
            upward_rank += *max_it;
        }

        return upward_rank;
    }
};
