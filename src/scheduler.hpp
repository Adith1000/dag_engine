#pragma once
#include "task.hpp"
#include <vector>
#include <functional> 

// Runs the tasks in order, respecting “must‑run‑before” deps and priorities.
// For now, executes each task by calling the provided `run_fn(task)`.
void run_scheduler(
    const std::vector<Task>& tasks,
    const std::function<void(const Task&)>& run_fn
);
