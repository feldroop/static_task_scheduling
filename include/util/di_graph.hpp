#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace util {

// simple directed graph, no delete functionality
// implemented using a hashmap based spare adjacency matrix

// V: vertex type, E: edge "weight" type
// E should be cheap to copy and store
template <typename V, typename E>
class di_graph {
public:
    using weight_matrix = std::vector<std::unordered_map<size_t, E>>;
private:
    // vertex id == vector index
    std::vector<V> vertices{};

    // current vertex id == vector index, neighbor id == hashmap key
    weight_matrix incoming_edges{};
    weight_matrix outgoing_edges{};

public:
    // returns id of newly created node
    size_t add_vertex(V const value) {
        size_t const id{vertices.size()};

        vertices.emplace_back(std::move(value));

        incoming_edges.emplace_back();
        outgoing_edges.emplace_back();

        return id;
    }

    // returns whether an edge was created
    bool add_edge(size_t const from_id, size_t const to_id, E const weight) {
        if (from_id >= vertices.size() ||
              to_id >= vertices.size() ||
            incoming_edges[to_id].contains(from_id)) {
            return false;
        }
        
        // copy weight for convenient handling and access
        incoming_edges[to_id][from_id] = weight;
        outgoing_edges[from_id][to_id] = std::move(weight);

        return true;
    }

    V const & get_vertex(size_t const id) const {
        return vertices.at(id);
    }

    std::vector<V> const & get_all_vertices() const {
        return vertices;
    }

    std::unordered_map<size_t, E> const & get_incoming_edges(size_t const id) const {
        return incoming_edges.at(id);
    }

    weight_matrix const & get_all_incoming_edges() const {
        return incoming_edges;
    }

    std::unordered_map<size_t, E> const & get_outgoing_edges(size_t const id) const {
        return outgoing_edges.at(id);
    }

    weight_matrix const & get_all_outgoing_edges() const {
        return outgoing_edges;
    }
};

} // namespace util
