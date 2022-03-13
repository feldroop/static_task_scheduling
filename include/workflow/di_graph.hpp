#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

// simple directed graph, no delete functionality
// implemented using a hashmap based spare adjacency matrix

// V: vertex type, E: edge "weight" type
// E should be cheap to copy and store
template <typename V, typename E>
class di_graph {
public:
    struct vertex {
        size_t const id;
        V const value;
    };

    using weight_matrix = std::vector<std::unordered_map<size_t, E>>;

private:
    // id == index
    std::vector<vertex> vertices{};

    // id0 == index, id1 == key
    weight_matrix incoming_edges{};
    weight_matrix outgoing_edges{};

    std::unordered_set<size_t> no_incoming{};
    std::unordered_set<size_t> no_outgoing{};

public:
    // returns id of newly created node
    size_t add_vertex(V const value) {
        size_t const id{vertices.size()};

        vertices.emplace_back(vertex{id, std::move(value)});

        incoming_edges.emplace_back();
        outgoing_edges.emplace_back();

        no_incoming.insert(id);
        no_outgoing.insert(id);

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
        
        no_incoming.erase(to_id);
        no_outgoing.erase(from_id);

        return true;
    }

    std::vector<vertex> const & get_vertices() const {
        return vertices;
    }

    weight_matrix const & get_incoming_edges() const {
        return incoming_edges;
    }

    weight_matrix const & get_outgoing_edges() const {
        return outgoing_edges;
    }

    std::unordered_set<size_t> const & get_no_incoming() const {
        return no_incoming;
    }

    std::unordered_set<size_t> const & get_no_outgoing() const {
        return no_outgoing;
    }
};
