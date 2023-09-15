#pragma once
#include "Socket/Socket.h"
#include "Thread/ThreadPool.hpp"
#include "Timer/Timer.h"
#include "pch.h"
#include <future>

namespace server {

enum EventType {NONE = 0x0, READ = 0x1, WRITE = 0x4};
class IOManager;
struct Fd {
    Fd() = default;
    Fd(IOManager* io): io_(io) {}
    struct Event {
        EventType type;
        std::function<void()> cb;
    };

    Event& getEvent(EventType type);
    void  triggerEvent(EventType type);
    void resetEvent(Event event);
    EventType events_{NONE};
    Event read;
    Event write;
    IOManager* io_;
};


class IOManager: public TimerManager {
public:
    using ptr = std::unique_ptr<IOManager>;
    friend class Fd;
    ~IOManager();

    static IOManager* getInstance();
    void loop();
    void tick();
    void AddEvent(int fd, EventType type, std::function<void()>  cb);
    void DelEvent(int fd, EventType type);
    // void CancalEvent(int fd);

    void SetFdsSize(size_t size);
    Fd* getFd(int fd);

    template<typename F, typename ...Args>
    auto enqueue(F&& f, Args&&... args) {
       return pool_.enqueue(f, args...);
    }
private:
    IOManager();
    static ptr ioManager_;
    std::vector<Fd*> fds_;
    ThreadPool pool_;
    int pipefd[2];
    int epollfd_;
    bool stop_;
    std::thread::id threadId_;
};
}