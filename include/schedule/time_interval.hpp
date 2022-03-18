#pragma once

#include <cstddef>

namespace schedule {

using time_t = double;

struct time_interval {
    // members can't be const because this needs to be move-assignable
    time_t start;
    time_t end;
    size_t task_id;
    size_t node_id;
};

} // namespace schedule
