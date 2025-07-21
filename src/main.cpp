#include "parser.hpp"
#include "task.hpp"
#include "scheduler.hpp"
#include "thread_pool.hpp"
#include "tracer.hpp"
#include "metrics.hpp"
#include "httplib.h"
#include <iostream>
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " tasks.json\n";
        return 1;
    }

    try {

        // Launch metrics endpoint on port 8080
        std::thread([]{
            httplib::Server svr;
            svr.Get("/metrics", [](auto&, auto& res){
                std::ostringstream oss;
                oss << "# HELP tasks_enqueued_total Total tasks enqueued\n";
                oss << "tasks_enqueued_total " << tasks_enqueued_total.load() << "\n";
                oss << "# HELP tasks_completed_total Total tasks finished\n";
                oss << "tasks_completed_total " << tasks_completed_total.load() << "\n";
                oss << "# HELP tasks_in_queue Current queue depth\n";
                oss << "tasks_in_queue " << tasks_in_queue.load() << "\n";
                oss << "# HELP threads_idle Number of idle threads\n";
                oss << "threads_idle " << threads_idle.load() << "\n";
                res.set_content(oss.str(), "text/plain");
            });
            svr.listen("0.0.0.0", 8080);
        }).detach();
        std::cout << "Metrics server listening on http://localhost:8080/metrics\n";


        // 1) Load tasks
        auto tasks = load_tasks(argv[1]);
        std::cout << "Loaded " << tasks.size() << " tasks:\n";
        Tracer tracer;
        tracer.init(tasks);
        
        // 2) Start the pool
        ThreadPool pool;
        unsigned int workers = std::thread::hardware_concurrency();
        pool.start(workers);
        std::cout << "Started thread-pool with " << workers << " workers\n";

        // 3) Time the run
        auto t0 = std::chrono::steady_clock::now();

        // 4) Run scheduler, enqueueing work into the pool
        run_scheduler(tasks, [&](const Task& t) {
            pool.enqueue([&, t] {
                tracer.set_state(t.id, TaskState::Running);

                // Simulate work—replace with your real task logic
                std::cout << "[RUN] " << t.id
                        << " on thread " << std::this_thread::get_id()
                        << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(1));

                tracer.set_state(t.id, TaskState::Finished);
            });
        });

        // 5) Shutdown pool (waits for all enqueued jobs)
        pool.shutdown();
        auto t1 = std::chrono::steady_clock::now();

        // 6) Report total time
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        std::cout << "Total execution time: " << ms << " ms\n";
        
        // 7) Use Graphviz to visualise the workflow
        tracer.dump_dot("dag.dot");
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }

    return 0;
}