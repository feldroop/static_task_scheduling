#pragma once

#include <algorithm>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <vector>

class cluster {
public:
    using node_id = size_t;

    struct cluster_node {
        node_id const id;
        size_t const memory;
        size_t const num_cores;
        double const core_performance;
        double const network_bandwidth;

        double performance() const {
            // assumes perfectly parallelizable tasks
            return core_performance * num_cores;
        }
    };

    using const_iterator = std::vector<cluster_node>::const_iterator;

private:
    std::vector<cluster_node> nodes{};

public:
    cluster(
        std::vector<size_t> const & memories,
        std::vector<size_t> const & num_cores,
        std::vector<double> const & core_performances,
        std::vector<double> const & bandwidths
    ) {
        if (
            memories.size() != num_cores.size() ||
            memories.size() != core_performances.size() ||
            memories.size() != bandwidths.size()
        ) {
            throw std::invalid_argument("Arguments for cluster parameters must be of the same size.");
        } else if (memories.empty()) {
            throw std::invalid_argument("Cluster cannot be empty.");
        }

        for (size_t i = 0; i < memories.size(); i++)
        {
            cluster_node n{i, memories[i], num_cores[i], core_performances[i], bandwidths[i]};
            nodes.push_back(n);
        }
    }

    double mean_node_performance() const {
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

    double best_node_performance() const {
        // safe dereference because cluster size enforced to be > 0
        return std::max_element(
            begin(),
            end(),
            [] (auto const & n0, auto const & n1) {
                return n0.performance() < n1.performance();
            }
        )->performance();
    }

    double mean_node_bandwidth() const {
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
