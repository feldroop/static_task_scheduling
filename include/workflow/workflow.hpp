#pragma once

#include <cassert>
#include <ranges>
#include <vector>

#include <util/di_graph.hpp>
#include <workflow/task.hpp>

namespace workflow {

using workflow = util::di_graph<task, double>;

// create a DAG workflow represetation based on the input specifications
workflow create_workflow(
    std::vector<double> const & computation_costs,
    std::vector<size_t> const & memory_requirements,
    std::vector<size_t> const & from_ids,
    std::vector<size_t> const & to_ids,
    std::vector<double> const & weights
) {
    workflow w{};
    std::vector<size_t> ids{};

    assert(computation_costs.size() == memory_requirements.size());

    for (size_t i = 0; i < computation_costs.size(); ++i) {
        task const t = task{i, computation_costs[i], memory_requirements[i]};
        size_t const id = w.add_vertex(t);
        ids.push_back(id);
    }

    assert(
        from_ids.size() == to_ids.size() &&
        to_ids.size() == weights.size()
    );

    for (size_t i = 0; i < from_ids.size(); ++i) {
        bool const was_created = w.add_edge(from_ids[i], to_ids[i], weights[i]);
        assert(was_created);
    }

    return w;
}

} //namepsace workflow
