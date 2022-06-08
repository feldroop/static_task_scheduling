#pragma once

#include <cluster/cluster.hpp>
#include <util/timepoint.hpp>

namespace workflow {

// data transfer cost without taking into account equal node ids
util::timepoint get_raw_data_transfer_cost(
    double const data_transfer,
    // for now, just use one bandwidth as they are assumed to be equal
    double const bandwidth
) {
    return data_transfer / bandwidth;
}

util::timepoint get_data_transfer_cost(
    cluster::node_id const node_id0,
    cluster::node_id const node_id1,
    double const data_transfer,
    double const bandwidth
) {
    // if the two tasks are scheduled to the same node, there is no cost for the data transfer
    if (node_id0 == node_id1) {
        return 0.0;
    }

    return get_raw_data_transfer_cost(data_transfer, bandwidth);
}

} // namespace workflow 
