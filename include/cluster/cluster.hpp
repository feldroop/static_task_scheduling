#pragma once

#include <algorithm>
#include <iostream>
#include <functional>
#include <numeric>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <cluster/cluster_node.hpp>

namespace cluster {

class cluster {
public:
    using const_iterator = std::vector<cluster_node>::const_iterator;

private:
    std::vector<cluster_node> const nodes{};

public:
    cluster(std::vector<cluster_node> const nodes_) 
    : nodes(std::move(nodes_)) 
    {}

    std::vector<node_id> node_ids_sorted_by_performance_descending() const {
        std::vector<node_id> node_ids(nodes.size());
        std::iota(node_ids.begin(), node_ids.end(), 0);

        std::ranges::sort(node_ids, std::ranges::greater(), [this] (node_id const & n_id) {
            return nodes.at(n_id).performance();
        });

        return node_ids;
    }

    node_id best_performance_node(double const memory_requirement = 0.0) const {
        auto valid_nodes = nodes 
            | std::views::filter([memory_requirement] (cluster_node const & node) {
                return node.memory >= memory_requirement;
            });

        // safe dereference because cluster size enforced to be > 0
        return std::ranges::max_element(
            valid_nodes,
            {},
            [] (cluster_node const & node) {
                return node.performance();
            }
        )->id;
    }

    double mean_performance() const {
        double const performance_sum = std::transform_reduce(
            begin(), 
            end(),
            0.0,
            std::plus<>(),
            [] (auto const & n) {
                return n.performance();
            }
        );

        return performance_sum / size();
    }

    double best_performance() const {
        // safe dereference because cluster size enforced to be > 0
        return std::ranges::max_element(
            nodes,
            {},
            [] (cluster_node const & node) {
                return node.performance();
            }
        )->performance();
    }

    double mean_bandwidth() const {
        double const performance_sum = std::transform_reduce(
            begin(), 
            end(),
            0.0,
            std::plus<>(),
            [] (auto const & n) {
                return n.network_bandwidth;
            }
        );

        return performance_sum / size();
    }

    std::string to_string() const {
        std::stringstream out;

        out << "########## Cluster: ##########\n";
        for(cluster_node const & node : nodes) {
            out << node.to_string() << '\n';
        }
        out << '\n';

        return out.str();
    }

    size_t size() const {
        return nodes.size();
    }

    const_iterator begin() const {
        return nodes.begin();
    }

    const_iterator end() const {
        return nodes.end();
    }
};

} // namespace cluster
