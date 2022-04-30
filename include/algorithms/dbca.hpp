#pragma once

#include <cluster/cluster.hpp>
#include <schedule/schedule.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {

// Dependency balance clustering algorithm

// Running time analysis:
// TODO

schedule::schedule dbca(
    cluster::cluster const & c, 
    workflow::workflow const & w,
    bool const use_memory_requirements,
    [[maybe_unused]] bool const verbose
) {
    schedule::schedule s(c, use_memory_requirements);
    return s;
}

}  // namespace algorithms
