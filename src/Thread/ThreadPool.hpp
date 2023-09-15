#pragma once

#include "pch.h"
#include <condition_variable>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

namespace server {
class ThreadPool {
public:
    explicit ThreadPool(size_t);
    ~ThreadPool();

    template<typename F, typename ...Args>
    auto enqueue(F&& f, Args&&... args);

    ThreadPool(const ThreadPool& ) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(const ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&&) = delete; 

private:
    std::vector<std::thread> workers;   // 线程池
    std::queue<std::function<void()>>  tasks; // 任务队列

    std::mutex mutex_;
    std::condition_variable cond_;
    bool stop_;
};


inline ThreadPool::ThreadPool(size_t threads): stop_(false) {
    for(size_t i = 0; i < threads; ++i) {
        workers.emplace_back(
            [this]() {
                for(;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> loc(this->mutex_);
                        this->cond_.wait(loc, [this](){ return this->stop_ || !this->tasks.empty();});
                        if(this->stop_ && this->tasks.empty()) {
                            return ;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            }
        );
    }
}

inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> loc(mutex_);
        stop_ = true;
    }
    
    cond_.notify_all();
    for(std::thread& thread : workers) {
        thread.join();
    }
}

template<typename F, typename...Args>
auto ThreadPool::enqueue(F &&f, Args &&...args) {
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#if _MSVC_LANG_ <= 201402L                   // c++14
    using return_type = typename std::result_of<F(Args...)>::type;
#else 
    using return_type = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
#endif
#elif defined (__linux__)
#if _cplusplus <= 201402L
using return_type = typename std::result_of<F(Args...)>::type;
#else 
    using return_type = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
#endif
#endif

    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> loc(mutex_);
        if(stop_) {
            throw std::runtime_error("don't allow enquene after stoping the pool");
        }   
        tasks.emplace([task]() {(*task)();});
    }
    cond_.notify_one();
    return res;
}


}