#pragma once

#include <workflow/task.hpp>

namespace workflow {

struct dependency {
    task_id const from_id;
    task_id const to_id;
};

} // namespace workflow
