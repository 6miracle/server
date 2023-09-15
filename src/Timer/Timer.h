#pragma once 
#include "pch.h"
#include <chrono>

namespace server {
class Timer:public std::enable_shared_from_this<Timer> {
public:
    using TimerCallback = std::function<void()>;
    Timer(uint64_t ms, TimerCallback callback, bool isRecurring = false);
    ~Timer() = default;

    // 重置执行时间
    void refresh(); 
    // 定时器执行
    void run();
    void cancel();

    int getTimeout() const noexcept { return ms_; }
    TimerCallback getCallBack()  const { return callback_; }

    std::chrono::steady_clock::time_point getTime() const { return begin_ + std::chrono::milliseconds(ms_); }
    bool operator<(const Timer&) const;
private:
    int ms_;     // 执行周期，即执行时间
    TimerCallback callback_;
    bool isRecurring_; // 是否循环执行
    std::chrono::steady_clock::time_point begin_;               // 开始时间
};

class TimerManager {
public:
    using TimerCallback = std::function<void()>;
    TimerManager() = default;
    void AddTimer(uint64_t ms, TimerCallback callback, bool isRecurring = false);
    void AddTimer(Timer& timer);
    void DelTimer(Timer& timer);
    void RefreshTimer(Timer& timer);
    void ListTimers(std::vector<std::function<void()>>&);

    bool empty() const { return timers_.empty();}
    std::chrono::time_point<std::chrono::steady_clock> getMinFin() const { return minFin_; }
    int getTimeout() const noexcept {return timeout; }
private:
    std::set<Timer> timers_;
    std::chrono::time_point<std::chrono::steady_clock> minFin_;  // 最短完成时间
    int timeout;
};
}