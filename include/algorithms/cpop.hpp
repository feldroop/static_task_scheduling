#pragma once 

#include <cluster/cluster.hpp>
#include <schedule/schedule.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {

// Running time analysis:
// TODO

schedule::schedule cpop(
    cluster::cluster const & c, 
    workflow::workflow const & w
) {
    schedule::schedule s(c);

    return s;
}

} // namespace algorithms
