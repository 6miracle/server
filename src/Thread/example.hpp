#pragma once
#include "pch.h"
#include <condition_variable>
#include <mutex>
#include <thread>

class ThreadPool {
public:
    ThreadPool(size_t num);
    ~ThreadPool();

    template<typename F, typename ...Args>
    bool enqueue(F&& f, Args&&... args);
private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> works_;
    std::condition_variable condition_;
    std::mutex mutex_;
};

inline ThreadPool::ThreadPool(size_t num) {
    for(size_t i = 0; i < num; ++i) {
        threads_.emplace_back([this]() {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this]() { return !works_.empty(); });
            auto work = works_.front();
            works_.pop();
            work();
        });
    }
}
template<typename F, typename ...Args>
bool ThreadPool::enqueue(F&& f, Args&&... args) {
    std::unique_lock<std::mutex> lock(mutex_);
    works_.emplace(std::bind(std::move(f), args...));
    
}
