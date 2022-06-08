#pragma once

#include <optional>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace util {

// simple directed graph, no delete functionality
// implemented using a hashmap based sparse adjacency matrix

// V: vertex type, W: edge "weight" type
// W should be cheap to copy and store
template <typename V, typename W>
class di_graph {
public:
    using vertex_id = size_t;
    using weight_matrix = std::vector<std::unordered_map<vertex_id, W>>;
    using vertex_iterator = std::vector<V>::const_iterator;

private:
    // vertex id == vector index
    std::vector<V> vertices{};

    // current vertex id == vector index, neighbor id == hashmap key
    weight_matrix incoming_edges{};
    weight_matrix outgoing_edges{};

public:
    // returns id of newly created vertex
    vertex_id add_vertex(V const value) {
        vertex_id const v_id{vertices.size()};

        vertices.emplace_back(std::move(value));

        incoming_edges.emplace_back();
        outgoing_edges.emplace_back();

        return v_id;
    }

    // returns whether an edge was created
    bool add_edge(vertex_id const from_id, vertex_id const to_id, W const weight) {
        if (from_id >= vertices.size() ||
              to_id >= vertices.size() ||
            incoming_edges.at(to_id).contains(from_id)) {
            return false;
        }
        
        // copy weight for convenient handling and access
        incoming_edges.at(to_id).insert({from_id, weight});
        outgoing_edges.at(from_id).insert({to_id, weight});

        return true;
    }

    V const & get_vertex(vertex_id const v_id) const {
        return vertices.at(v_id);
    }

    std::vector<V> const & get_all_vertices() const {
        return vertices;
    }

    std::unordered_map<vertex_id, W> const & get_incoming_edges(vertex_id const v_id) const {
        return incoming_edges.at(v_id);
    }

    weight_matrix const & get_all_incoming_edges() const {
        return incoming_edges;
    }

    std::unordered_map<vertex_id, W> const & get_outgoing_edges(vertex_id const v_id) const {
        return outgoing_edges.at(v_id);
    }

    weight_matrix const & get_all_outgoing_edges() const {
        return outgoing_edges;
    }

    // returns vertices without incoming edges ("independent vertices")
    std::unordered_set<vertex_id> get_independent_vertex_ids() const {
        std::unordered_set<vertex_id> independent_vertex_ids{};

        for (vertex_id v_id = 0; v_id < vertices.size(); ++v_id) {
            if (incoming_edges.at(v_id).empty()) {
                independent_vertex_ids.insert(v_id);
            }
        }

        return independent_vertex_ids;
    }

    // returns a std::nullopt if the graph is cyclic
    // running time: linear in the number of edges
    std::optional<std::vector<vertex_id>> topological_order() const {
        std::vector<vertex_id> topological_order{};
        std::unordered_set<vertex_id> independent_vertex_ids = get_independent_vertex_ids();

        // copy incoming edges to modify
        auto temp_incoming_edges = incoming_edges;

        // keep on finding "independent" vertices without incoming edges
        // then extract them and delete their outgoing edges to make other vertices independent
        while (!independent_vertex_ids.empty()) {
            vertex_id const curr_vertex_id = *independent_vertex_ids.begin();
            independent_vertex_ids.erase(curr_vertex_id);
            topological_order.push_back(curr_vertex_id);

            for (auto const & [neighbor_id, weight] : outgoing_edges.at(curr_vertex_id)) {
                size_t const num_erased = temp_incoming_edges.at(neighbor_id).erase(curr_vertex_id);
                if (num_erased != 1) {
                    return std::nullopt;
                }

                if (temp_incoming_edges.at(neighbor_id).empty()) {
                    independent_vertex_ids.insert(neighbor_id);
                }
            }
        }

        if (topological_order.size() != vertices.size()) {
            return std::nullopt;
        }

        return topological_order;
    }
};

} // namespace util
