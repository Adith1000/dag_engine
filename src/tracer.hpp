#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "parser.hpp"

enum class TaskState { NotStarted, Running, Finished };

class Tracer {
public:
    void init(const std::vector<Task>& tasks);
    void set_state(const std::string& id, TaskState state);
    void dump_dot(const std::string& filename) const;

private:
    struct Node {
        std::string id;
        int priority;
        std::vector<std::string> deps;
        TaskState state = TaskState::NotStarted;
    };

    std::unordered_map<std::string, Node> task_map;
};
