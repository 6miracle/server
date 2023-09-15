#include "Timer.h"
#include <chrono>
#include <ratio>
#include <time.h>
namespace server {
static std::chrono::steady_clock::time_point GetCurrentMs() {
    return std::chrono::steady_clock::now();
}
Timer::Timer(uint64_t ms, TimerCallback callback, bool isRecurring):
    ms_(ms), callback_(callback), isRecurring_(isRecurring), begin_(GetCurrentMs()) {}

void Timer::run() {
    if(!callback_) {
        callback_();
    }
}
void Timer::refresh() {
    begin_ = GetCurrentMs();
}

void Timer::cancel() {
    isRecurring_ = false;
    callback_ = NULL;
}

bool Timer::operator<(const Timer& timer) const {
    return getTime() < timer.getTime();
}

void TimerManager::AddTimer(uint64_t ms, Timer::TimerCallback callback, bool isRecurring) {
    Timer timer(ms, callback, isRecurring);
    AddTimer(timer);
}

void TimerManager::AddTimer(Timer &timer) {
    if(timers_.empty() || timer.getTime() < minFin_) {
        minFin_ = timer.getTime();
        timeout = timer.getTimeout();
    }
    timers_.emplace(timer);
}
void TimerManager::DelTimer(Timer &timer) {
    timers_.erase(timer);
    if(timer.getTime() == minFin_ && !timers_.empty()) {
        minFin_ = timers_.begin()->getTime();
        timeout = timers_.begin()->getTimeout();
    }
}

// 列出所有满足条件的定时器
void TimerManager::ListTimers(std::vector<std::function<void()>>& cbs) {
    if(timers_.empty() || minFin_ < GetCurrentMs()) { return ; }
    auto iter = timers_.begin();
    while(iter->getTime() <= GetCurrentMs()) {
        cbs.emplace_back(iter->getCallBack());
        ++iter;
    }
    
    if(iter != timers_.end()) { minFin_ = iter->getTime(); timeout = iter->getTimeout();}
    timers_.erase(timers_.begin(), iter);
}
void TimerManager::RefreshTimer(Timer &timer) {
    DelTimer(timer);
    timer.refresh();
    AddTimer(timer);
}
}