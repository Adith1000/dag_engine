#pragma once
#include <atomic>

// Total tasks submitted to the pool
extern std::atomic<size_t> tasks_enqueued_total;

// Total tasks that have finished execution
extern std::atomic<size_t> tasks_completed_total;

// Current number of tasks waiting in the queue
extern std::atomic<size_t> tasks_in_queue;

// Current number of idle worker threads
extern std::atomic<size_t> threads_idle;
