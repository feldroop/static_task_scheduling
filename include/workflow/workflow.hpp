#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <util/di_graph.hpp>
#include <util/timepoint.hpp>
#include <workflow/data_transfer_cost.hpp>
#include <workflow/node_task_matrix.hpp>
#include <workflow/task.hpp>
#include <workflow/task_dependency.hpp>

namespace workflow {

class workflow {
public:
    using iterator = util::di_graph<task, double>::vertex_iterator;

private:
    util::di_graph<task, double> g;
    std::vector<task_id> topological_task_order;
    std::vector<size_t> topological_task_ranks; // inverse of topological_task_order
    std::unordered_set<task_id> independent_task_ids;
    std::vector<std::vector<task_id>> const task_ids_per_bag;

public:
    // create a DAG workflow represetation based on the input specifications
    // it is assumed that the ids in from_ids and to_ids refer to the indices of the other arguments
    workflow(
        std::vector<task> tasks,
        std::vector<double> input_data_sizes,
        std::vector<double> output_data_sizes,
        std::vector<task_dependency> dependencies,
        std::vector<std::vector<task_id>> const task_ids_per_bag_
    ) : topological_task_ranks(tasks.size()), task_ids_per_bag(std::move(task_ids_per_bag_)) {
        if (
            tasks.size() != input_data_sizes.size()
            || tasks.size() != output_data_sizes.size()
        ) {
            throw std::invalid_argument("Arguments for task parameters must have the same size.");
        }

        for (task const & t : tasks) {
            if (t.workload == 0) {
                throw std::invalid_argument("All tasks need a workload > 0");
            }

            g.add_vertex(t);
        }

        for (task_dependency const & dep : dependencies) {
            double const output_data_size = output_data_sizes.at(dep.from_id);
            double const input_data_size = input_data_sizes.at(dep.to_id);

            if (output_data_size != input_data_size) {
                std::stringstream out{};
                out << "Input/Output data sizes for a dependency don't match. "
                    << dep.from_id << " -> " << input_data_size <<  '/'
                    << dep.to_id << " -> " << output_data_size;
                throw std::invalid_argument(out.str());
            }

            bool const was_created = g.add_edge(
                dep.from_id, 
                dep.to_id, 
                output_data_sizes.at(dep.from_id)
            );

            if (!was_created) {
                throw std::invalid_argument("Task ids for dependency endpoints are invalid.");
            }
        }

        independent_task_ids = g.get_independent_vertex_ids();
        
        topological_task_order = g.topological_order().value();
        for (size_t const i : std::ranges::iota_view{0ul, size()}) {
            topological_task_ranks.at(topological_task_order.at(i)) = i;
        }
    }

    // performance and bandwidth are mean values for HEFT/CPOP
    // and uniform/best for TDCA
    std::unordered_map<task_id, double> all_downward_ranks(
        double const performance,
        double const bandwidth
    ) const {
        std::unordered_map<task_id, double> downward_ranks{};
        
        for (task_id const t_id : topological_task_order) {
            double const downward_rank = compute_downward_rank(
                downward_ranks, 
                performance, 
                bandwidth, 
                t_id
            );
            downward_ranks.insert({t_id, downward_rank});
        }

        return downward_ranks;
    }

    // performance and bandwidth are mean values for HEFT/CPOP
    // and uniform/best for TDCA
    std::unordered_map<task_id, double> all_upward_ranks(
        double const performance,
        double const bandwidth
    ) const {
        std::unordered_map<task_id, double> upward_ranks{};
        
        for (task_id const t_id : std::views::reverse(topological_task_order)) {
            double const upward_rank = compute_upward_rank(
                upward_ranks, 
                performance, 
                bandwidth, 
                t_id
            );
            upward_ranks.insert({t_id, upward_rank});
        }

        return upward_ranks;
    }

    std::tuple<
        node_task_matrix<util::timepoint>, 
        node_task_matrix<util::timepoint>,
        std::vector<task_id>
    > // return: (est, eft, cpred)
    compute_est_and_eft(cluster::cluster const & c) const {
        using util::timepoint;
        std::vector<std::vector<timepoint>> est_data(c.size(), std::vector<timepoint>(size()));
        std::vector<std::vector<timepoint>> eft_data(c.size(), std::vector<timepoint>(size()));
        
        std::vector<task_id> cpred(size());
        cluster::node_id const best_node_id = c.best_performance_node();

        for (task_id const & t_id : topological_task_order) {
            for (cluster::cluster_node const & node : c) {
                auto & curr_est = est_data.at(node.id).at(t_id);
                auto & curr_eft = eft_data.at(node.id).at(t_id);

                auto incoming_efts = get_task_incoming_edges(t_id)
                    | std::views::transform([&c, &eft_data, best_node_id, node] (auto const & edge) {
                        auto const & [neighbor_id, data_transfer] = edge;

                        // since in our model the node performances simple scale the task workloads,
                        // the node with the best eft will always be the one with the best performance
                        timepoint const pred_eft_best = eft_data.at(best_node_id).at(neighbor_id);
                        timepoint const data_transfer_cost_best = get_data_transfer_cost(
                            best_node_id,
                            node.id,
                            data_transfer,
                            c.uniform_bandwidth()
                        );

                        // the next est could still be improved by keeping the tasks on the same node
                        timepoint const pred_eft_same = eft_data.at(node.id).at(neighbor_id);
                        timepoint const data_transfer_cost_same = get_data_transfer_cost(
                            node.id,
                            node.id,
                            data_transfer,
                            c.uniform_bandwidth()
                        );

                        timepoint const best_incoming_eft = std::min(
                            pred_eft_best + data_transfer_cost_best,
                            pred_eft_same + data_transfer_cost_same
                        );

                        return std::make_tuple(
                            best_incoming_eft,
                            neighbor_id
                        );
                    });

                auto const max_it = std::ranges::max_element(
                    incoming_efts,
                    {},
                    [] (auto const & tup) {
                        // get incoming eft timepoint
                        return std::get<0>(tup);
                    }
                );

                auto const [max_incoming_eft, cpred_id] = max_it == incoming_efts.end() 
                    ? std::make_tuple(0.0, std::numeric_limits<task_id>::max())
                    : *max_it;

                curr_est = max_incoming_eft;
                curr_eft = curr_est + get_task(t_id).workload / node.performance();

                if (node.id == best_node_id) {
                    cpred.at(t_id) = cpred_id;
                }
            }
        }

        return std::make_tuple(
            node_task_matrix(std::move(est_data)),
            node_task_matrix(std::move(eft_data)),
            std::move(cpred)
        );
    }

    util::timepoint get_sequential_makespan(double const best_cluster_node_performance) const {
        auto const & tasks = g.get_all_vertices();
        
        return std::transform_reduce(
            tasks.begin(),
            tasks.end(), 
            0.0,
            std::plus<>(),
            [best_cluster_node_performance] (auto const & t) {
                return t.workload / best_cluster_node_performance;
            }
        );
    }

    std::string to_string(std::optional<double> best_performance_opt = std::nullopt) const {
        std::stringstream out;

        out << "########## Workflow: ##########\n";
        out << "-- dependency format: (-> <target_task_id>, <data_transfer>)\n";
        
        for (task const & t : g.get_all_vertices()) {
            out << "task " << t.id 
                << ": workload " << t.workload
                << ", memory " << t.memory_requirement
                << ",\n\toutgoing dependencies:";

            for (auto const & [neighbor_id, data_transfer] : g.get_outgoing_edges(t.id)) {
                out << " (-> " << neighbor_id << ", " << data_transfer << ')';
            }

            out << '\n';
        }

        if (best_performance_opt) {
            out << "sequential makespan: " 
                << get_sequential_makespan(best_performance_opt.value())
                << '\n';
        }

        out << '\n';

        return out.str();
    }

    std::vector<task_id> const & get_task_topological_order() const {
        return topological_task_order;
    }

    size_t topological_task_rank(task_id const t_id) const {
        return topological_task_ranks.at(t_id);
    }

    std::unordered_set<task_id> const & get_independent_task_ids() const {
        return independent_task_ids;
    }

    std::vector<std::vector<task_id>> const & get_task_ids_per_bag() const {
        return task_ids_per_bag;
    }

    std::unordered_map<task_id, double> const & get_task_incoming_edges(task_id const t_id) const {
        return g.get_incoming_edges(t_id);
    }

    std::unordered_map<task_id, double> const & get_task_outgoing_edges(task_id const t_id) const {
        return g.get_outgoing_edges(t_id);
    }

    util::di_graph<task, double>::weight_matrix const & get_all_incoming_edges() const {
        return g.get_all_incoming_edges();
    }

    util::di_graph<task, double>::weight_matrix const & get_all_outgoing_edges() const {
        return g.get_all_outgoing_edges();
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
        double const performance,
        double const bandwidth,
        task_id const t_id
    ) const {
        double upward_rank = get_task(t_id).workload / performance;

        auto outgoing_ranks = g.get_outgoing_edges(t_id) 
            | std::views::transform([&upward_ranks, bandwidth] (auto const & edge) {
                auto const & [neighbor_id, data_transfer] = edge;
                return data_transfer / bandwidth + upward_ranks.at(neighbor_id);
            }
        );

        auto const max_it = std::ranges::max_element(outgoing_ranks);
        if (max_it != outgoing_ranks.end()) {
            upward_rank += *max_it;
        }

        return upward_rank;
    }

    double compute_downward_rank(
        std::unordered_map<task_id, double> const & downward_ranks,
        double const performance,
        double const bandwidth,
        task_id const t_id
    ) const {
        auto incoming_ranks = g.get_incoming_edges(t_id) 
            | std::views::transform([this, &downward_ranks, performance, bandwidth] (auto const & edge) {
                auto const & [neighbor_id, data_transfer] = edge;
                double const neighbor_compute_cost = g.get_vertex(neighbor_id).workload / performance;
                double const data_transfer_cost = data_transfer / bandwidth;
                return  neighbor_compute_cost + data_transfer_cost + downward_ranks.at(neighbor_id);
            }
        );

        auto const max_it = std::ranges::max_element(incoming_ranks);
        if (max_it == incoming_ranks.end()) {
            return 0.0;
        }

        return *max_it;
    }
};

} // namespace workflow
