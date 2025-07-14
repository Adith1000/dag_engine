#pragma once
#include <string>
#include <vector>

struct Task {
    std::string id;
    int priority;
    std::vector<std::string> deps;
};