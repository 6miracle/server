#pragma once

#include <mutex>
#include <shared_mutex>

class RWLock {
public:
    RWLock() = default;
    ~RWLock() = default;
    RWLock(const RWLock&) = delete;
    RWLock operator=( const RWLock&) = delete;

    void WLock()    { mutex_.lock(); }
    void WUnlock()  { mutex_.unlock();}
    void RLock()    { mutex_.lock_shared(); }
    void RUnlock()  { mutex_.unlock_shared();}

private:
    std::shared_mutex mutex_;
};