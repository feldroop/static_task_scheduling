#pragma once

#include <workflow/task.hpp>

namespace workflow {

struct task_dependency {
    // these cannot be const for copy/move assignment
    task_id from_id;
    task_id to_id;
};

} // namespace workflow
