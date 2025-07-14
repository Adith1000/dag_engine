#include "scheduler.hpp"
#include <unordered_map>
#include <queue>
#include <functional>
#include <stdexcept>

// Comparison for highest‑priority first
struct TaskCompare {
    bool operator()(const Task* a, const Task* b) const {
        return a->priority < b->priority;
    }
};

void run_scheduler(
    const std::vector<Task>& tasks,
    const std::function<void(const Task&)>& run_fn
) {
    // 1) Map task IDs to Task*
    std::unordered_map<std::string, const Task*> task_map;
    for (auto& t : tasks) {
        task_map[t.id] = &t;
    }

    // 2) Build adjacency list and indegree count
    std::unordered_map<std::string, std::vector<std::string>> adj;
    std::unordered_map<std::string, int> indegree;
    for (auto& t : tasks) {
        indegree[t.id];  // ensure key exists, defaults to 0
    }
    for (auto& t : tasks) {
        for (auto& dep : t.deps) {
            if (!task_map.count(dep))
                throw std::runtime_error("Unknown dependency: " + dep);
            adj[dep].push_back(t.id);
            indegree[t.id]++;
        }
    }

    // 3) Initialize ready queue with indegree == 0
    std::priority_queue<const Task*, std::vector<const Task*>, TaskCompare> ready;
    for (auto& [id, deg] : indegree) {
        if (deg == 0) {
            ready.push(task_map[id]);
        }
    }

    // 4) Process tasks
    int processed = 0;
    while (!ready.empty()) {
        const Task* cur = ready.top();
        ready.pop();

        // “Run” the task via callback
        run_fn(*cur);
        processed++;

        // Decrement indegree of dependents
        for (auto& nbr_id : adj[cur->id]) {
            if (--indegree[nbr_id] == 0) {
                ready.push(task_map[nbr_id]);
            }
        }
    }

    // 5) Check for cycles
    if (processed != (int)tasks.size()) {
        throw std::runtime_error("Cycle detected or missing dependencies");
    }
}
 