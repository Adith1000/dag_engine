#include "thread_pool.hpp"
#include "metrics.hpp"            
#include <thread>
#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace {
    // Shared state for the pool:
    std::deque<std::function<void()>> jobs;
    std::vector<std::thread> threads;
    std::mutex mtx;
    std::condition_variable cv;
    bool stopping = false;

    // Worker loop run by each thread
    void worker_loop() {
        while (true) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(mtx);
                // Wake on new job or shutdown
                cv.wait(lock, []{ return stopping || !jobs.empty(); });
                if (stopping && jobs.empty())
                    break;
                
                // Before popping, decrement the in‑queue counter
                tasks_in_queue--;

                job = std::move(jobs.front());
                jobs.pop_front();

                // A thread is now busy
                threads_idle--;
            }
            
            // Execute the job outside the lock
            job();

            // After completion, update completion counter and mark thread idle again
            tasks_completed_total++;
            threads_idle++;
        }
    }
}

void ThreadPool::start(size_t num_threads) {
    stopping = false;                // reset in case pool is reused
    
    // Initially, all threads are idle
    threads_idle = num_threads;

    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker_loop);
    }
}

void ThreadPool::enqueue(std::function<void()> job) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        // Enqueue the job and update metrics
        jobs.push_back(std::move(job));
        tasks_enqueued_total++;
        tasks_in_queue++;
    }
    cv.notify_one();
}

void ThreadPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        stopping = true;
    }
    cv.notify_all();
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    threads.clear();
}
