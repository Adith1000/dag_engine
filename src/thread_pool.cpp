#include "thread_pool.hpp"
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
                cv.wait(lock, []{ return stopping || !jobs.empty(); });
                if (stopping && jobs.empty())
                    break;
                job = std::move(jobs.front());
                jobs.pop_front();
            }
            job();
        }
    }
}

void ThreadPool::start(size_t num_threads) {
    stopping = false;             // reset in case pool is reused
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker_loop);
    }
}

void ThreadPool::enqueue(std::function<void()> job) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        jobs.push_back(std::move(job));
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
