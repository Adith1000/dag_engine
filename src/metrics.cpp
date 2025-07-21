#include "metrics.hpp"
std::atomic<size_t> tasks_enqueued_total{0};
std::atomic<size_t> tasks_completed_total{0};
std::atomic<size_t> tasks_in_queue{0};
std::atomic<size_t> threads_idle{0};
