#pragma once

#include <cstddef>

#include <util/timepoint.hpp>
#include <cluster/cluster.hpp>

namespace schedule {

using scheduled_task_id = size_t;

struct time_interval {
    // members can't be const because this needs to be move-assignable
    util::timepoint start;
    util::timepoint end;
    scheduled_task_id task_id;
    cluster::node_id node_id;
};

} // namespace schedule
