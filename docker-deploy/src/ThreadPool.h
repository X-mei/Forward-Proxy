#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
// Not sure how these's library function yet
#include <future>
#include <functional>
#include <stdexcept>
#include <vector>
#include <memory>

class ThreadPool {
private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;
    // the task queue, workers access it to fetch task
    std::queue<std::function<void()>> tasks;
    // lock used to make sure synchronized access to the task queue
    std::mutex queue_mutex;
    // conditional variable used to notify worker (have them awaked to do work/terminate)
    std::condition_variable condition;
    // ###### not sure why exist for know ######
    bool stop;
public:
    ThreadPool(size_t size);
    template <class F, class... Args>
    // the function to add new task to the task queue (core)
    auto enqueue(F && f, Args && args) -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();
};

inline ThreadPool::ThreadPool(size_t size):stop(false){
    workers.emplace_back([this]{
        for (;;) {// ##### similar to while(1), diff not sure #####
            std::function<void()> task;
            // ##### all enclosed is critical section? I assume #####
            {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                // if stoped or no task to fetch, block.
                // ##### not understanding the syxtax here #####
                this->condition.wait(lock, [this]{return this->stop() || !this->tasks.empty();})
                // after waked, only terminate worker thread when all tasks are done.
                if (this->stop && this->tasks.empty()){
                    return;
                }
                // use move semantic to gaurantee rvalue?
                // ##### why? #####
                task = std::move(this->tasks.front());
                this->tasks.pop();
            }
            task();
        }
    });
}

#endif