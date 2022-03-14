#pragma once

#include <cassert>
#include <ranges>

#include <workflow/di_graph.hpp>

template<typename V, typename E>
std::vector<size_t> find_independent_vertices(di_graph<V, E> const & g) {
    std::vector<size_t> independent_vertex_ids{};
    auto const & incoming_edges = g.get_all_incoming_edges();

    auto has_no_edges = [] (auto const & edges) {
        return edges.empty();
    };

    auto it = incoming_edges.cbegin();

    while (true)
    {
        it = std::ranges::find_if(it, incoming_edges.cend(), has_no_edges);
        
        if (it == incoming_edges.cend())
        {
            break;
        }

        size_t const vertex_id = it - incoming_edges.cbegin();
        independent_vertex_ids.push_back(vertex_id);
        ++it;
    }

    return independent_vertex_ids;
}

// Running time for G = (V,E) is in O(|E|)
template<typename V, typename E>
std::vector<size_t> compute_topological_order(di_graph<V, E> const & g) {
    std::vector<size_t> topological_order{};

    // copy incoming edges to modify later
    auto incoming_edges = g.get_all_incoming_edges();
    auto const & outgoing_edges = g.get_all_outgoing_edges();

    std::vector<size_t> independent_vertex_ids = find_independent_vertices(g);

    // keep on finding "independent" vertices without incoming edges
    // then extract them and delete their outgoing edges to free other vertices
    while (!independent_vertex_ids.empty()) {
        size_t const curr_vertex_id = independent_vertex_ids.back();
        independent_vertex_ids.pop_back();
        topological_order.push_back(curr_vertex_id);

        for (auto const & [neighbor_id, _] : outgoing_edges[curr_vertex_id])
        {
            size_t const num_erased = incoming_edges.at(neighbor_id).erase(curr_vertex_id);
            assert(num_erased == 1);

            if (incoming_edges.at(neighbor_id).empty()) {
                independent_vertex_ids.push_back(neighbor_id);
            }
        }
    }

    assert(topological_order.size() == g.get_all_vertices().size());

    return topological_order;
}