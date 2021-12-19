#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>
// Not sure how these's library function yet
#include <future>
#include <functional>
#include <stdexcept>
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
    auto enqueue(F && f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();
};

inline ThreadPool::ThreadPool(size_t size):stop(false){
    for(size_t i = 0; i < size ; ++i){
        workers.emplace_back([this]{
            for (;;) {// ##### similar to while(1), diff not sure #####
                std::function<void()> task;
                // by enclosing the section in a {}, the section acts like a function, that is, local
                // variables destruct themselves when going out of scope. Unique_lock is a templated 
                // class that offer RAII trait so when } is reached, the lock is automatically unlocked.
                // See more reference here: https://changkun.de/modern-cpp/zh-cn/07-thread/index.html
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    // if stoped or no task to fetch, block.
                    // ##### not understanding the syxtax here #####
                    this->condition.wait(lock, [this]{return this->stop || !this->tasks.empty();});
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
    
}

// Templated to handle different function and arguments
template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>{
    // use return_type instead of typing that all the time
    using return_type = typename std::result_of<F(Args...)>::type;
    // create a smart pointer of a packaged task to better track the execution of task
    // ##### details of execution not clear though #####
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    // define res as the result acquired asynchronously
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop){
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks.emplace([task](){ (*task)(); });
    }
    // call up one thread to handle the task
    condition.notify_one();
    // return the result immediately, but the actual completion is asynchronous(std::future)
    return res;
}

inline ThreadPool::~ThreadPool() {
    // set the stop indicator to true, protect it (singleton)
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    // wake up all the worker thread
    condition.notify_all();
    // we can join them directly since worker thread won't terminate before all task in queue is finished
    for (std::thread &worker: workers){
        worker.join();
    }
}

#endif