#pragma once

#include <vector>

#include <workflow/dependency.hpp>
#include <workflow/task_bag.hpp>
#include <workflow/topology/bag_dependency.hpp>
#include <workflow/topology/topology.hpp>

namespace workflow::topology {

std::vector<dependency> infer_dependencies(topology const top, std::vector<task_bag> const & task_bags) {
    // TODO
    return {};
}

} // namespace workflow::topology
