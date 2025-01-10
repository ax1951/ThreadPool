/*
Copyright (c) 2012 Jakob Progsch, Václav Zeman

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.

https://github.com/progschj/ThreadPool/tree/master

*/
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <functional>
#include <future>
#include <queue>
#include <thread>
#include <vector>
#include <type_traits>
#include <mutex>

class ThreadPool {
public:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    inline ThreadPool(uint32_t numberOfThreads, uint32_t maxNumberOfTasks);
    inline ~ThreadPool();

    template <class F, class... Args>
    std::future<std::invoke_result_t <F(Args...)>> enqueue(F&& f, Args &&...args);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    const uint32_t maxNumberOfTasks;
    bool stop;

    std::mutex queueMutex;
    std::condition_variable queueIsNotEmpty;
    std::condition_variable queueIsNotFull;
};


inline ThreadPool::ThreadPool(uint32_t numberOfThreads, uint32_t maxNumberOfTasks) :
    maxNumberOfTasks(maxNumberOfTasks),
    stop(false) {
    if (0 == numberOfThreads) {
        throw std::invalid_argument("Thread number must be greater than 0!");
    }

    workers.reserve(numberOfThreads);

    for (size_t i = 0; i < numberOfThreads; ++i) {
        std::thread worker([this]() {
            while (true) {
                std::function<void()> task;

                // pop a task from queue
                {
                    std::unique_lock lock(queueMutex);
                    queueIsNotEmpty.wait(lock, [this]() {
                        return stop || !tasks.empty();
                        });

                    // all tasks are completed, safe to exit
                    if (stop && tasks.empty())
                        return;

                    // even if stop = 1, once tasks is not empty, then
                    // excucte the task until tasks queue become empty
                    //
                    task = std::move(tasks.front());
                    tasks.pop();
                    queueIsNotFull.notify_one();
                }

                // execute the task
                task();
            }
            });

        workers.emplace_back(std::move(worker));
    }
}

inline ThreadPool::~ThreadPool() {
    // stop thread pool, and notify all threads to finish the remaining tasks.
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }

    queueIsNotEmpty.notify_all();

    for (auto& worker : workers)
        worker.join();
}

template <class F, class... Args>
std::future<std::invoke_result_t<F(Args...)>> ThreadPool::enqueue(F&& f, Args&&... args) {
    // The return type of task F with arguments args
    using return_type = std::invoke_result_t<F(Args...)>;

    // wrapper for no arguments
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock lock(queueMutex);

        if (stop) {
            throw std::runtime_error("The thread pool has been stopped!");
        }

        if (tasks.size() >= maxNumberOfTasks) {
            queueIsNotFull.wait(lock, [&]() { return tasks.size() < maxNumberOfTasks; });
        }

        // wrapper for no returned value
        tasks.emplace([task]() -> void { (*task)(); });
    }

    queueIsNotEmpty.notify_one();
    return res;
}

#endif
