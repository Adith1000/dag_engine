#include "parser.hpp"
#include "task.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " tasks.json\n";
        return 1;
    }

    try {
        auto tasks = load_tasks(argv[1]);
        std::cout << "Loaded " << tasks.size() << " tasks:\n";
        for (auto& t : tasks) {
            std::cout << "  - " << t.id
                      << " (priority=" << t.priority 
                      << ", deps=" << t.deps.size() << ")\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }

    return 0;
}
