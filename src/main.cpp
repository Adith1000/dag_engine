#include "parser.hpp"
#include "task.hpp"
#include "scheduler.hpp"
#include "thread_pool.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " tasks.json\n";
        return 1;
    }

    try {
        // 1) Load tasks
        auto tasks = load_tasks(argv[1]);
        std::cout << "Loaded " << tasks.size() << " tasks:\n";
        
        // 2) Start the pool
        ThreadPool pool;
        unsigned int workers = std::thread::hardware_concurrency();
        pool.start(workers);
        std::cout << "Started thread-pool with " << workers << " workers\n";

        // 3) Time the run
        auto t0 = std::chrono::steady_clock::now();

        // 4) Run scheduler, enqueueing work into the pool
        run_scheduler(tasks, [&](const Task& t) {
            pool.enqueue([t] {
                // Simulate work—replace with your real task logic
                std::cout << "[RUN] " << t.id
                        << " on thread " << std::this_thread::get_id()
                        << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            });
        });

        // 5) Shutdown pool (waits for all enqueued jobs)
        pool.shutdown();
        auto t1 = std::chrono::steady_clock::now();

        // 6) Report total time
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        std::cout << "Total execution time: " << ms << " ms\n";
        
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }

    return 0;
}