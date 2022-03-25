#pragma once

#include <cstddef>

#include <util/timepoint.hpp>

namespace schedule {

struct time_interval {
    // members can't be const because this needs to be move-assignable
    util::timepoint start;
    util::timepoint end;
    size_t task_id;
    size_t node_id;
};

} // namespace schedule
