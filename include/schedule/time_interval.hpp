#pragma once

#include <cstddef>

namespace schedule { 

struct time_interval {
    // members can't be const because this needs to be move-assignable
    double start;
    double end;
    size_t task_id;
};

} // namespace schedule
