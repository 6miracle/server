#include "IOManager.h"
#include "Logger/logger.hpp"
#include "Timer/Timer.h"
#include "common.h"
#include "config/config.h"
#include <asm-generic/errno-base.h>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <sys/epoll.h>
#include <thread>
namespace server {
IOManager::ptr IOManager::ioManager_;
size_t threadNum = 10;
// auto eventNum = Config::LookUp("EventNum", 1024, "Max Epoll Event Num");
// size_t threadNum = 10;
size_t eventNum = 1024;
// 设置为非阻塞
static void setNonblock(int fd) {
    int old = fcntl(fd, F_GETFL);
    int new_ = old | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_);
}

static void addFd(int epollfd, int fd, int status) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLET | status ;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNonblock(fd);
}
Fd::Event& Fd::getEvent(EventType type) {
    switch(type) {
        case EventType::READ:
            return read;
        case EventType::WRITE:
            return write;
        default:
            ASSERT(false, "Event Type Error");
            return read;
    }
}
// 触发事件
void  Fd::triggerEvent(EventType type) {
    if(type == EventType::NONE) return ;
    Event& event = getEvent(type);
    events_ = (EventType)(events_ & ~type);
    if(event.cb) { 
        // LOG_INFO("fd = %d trigger event = %d", )
        io_->pool_.enqueue(event.cb);
    } else {
        io_->pool_.enqueue(std::bind(&IOManager::tick, io_));
    }

}
void Fd::resetEvent(Event event) {
    event.cb = nullptr;
}

IOManager* IOManager::getInstance() {
    if(!ioManager_) {
        // std::cout<< "===================" <<'\n';
        ioManager_.reset(new IOManager());
    } 
    return ioManager_.get();
}

IOManager::IOManager(): pool_(10), stop_(false){
    ASSERT(pipe(pipefd) == 0, "pipe create fail");
    // printf("IOMANAGER %ld----%ld\n", &pipefd[0], &pipefd[1]);
    // 创建epoll
    epollfd_ = epoll_create(5);
    ASSERT(epollfd_ != -1, "epoll_create ERROR");
    // LOG_INFO("fd0 = %d, fd1 = %d", pipefd[0], pipefd[1]);
    addFd(epollfd_, pipefd[0], EPOLLIN);
    SetFdsSize(32);
    // pool_.enqueue(std::bind(&IOManager::loop, this));
    pool_.enqueue(std::bind(&IOManager::tick, this));
}

IOManager::~IOManager() {
    stop_ = true;
    close(epollfd_);
    close(pipefd[1]);
    close(pipefd[0]);
}
void IOManager::SetFdsSize(size_t size) {
    int before = fds_.size();
    fds_.resize(size);
    for(size_t i = before; i < size; ++i) {
        fds_[i] = new Fd(this);
    }
}
 
void IOManager::loop() {
    // printf("%ld----%ld\n", &pipefd[0], &pipefd[1]);
    threadId_ = std::this_thread::get_id();
    struct epoll_event events[eventNum];
    while(true) {
    //    LOG_INFO("epoll_wait");
        int tmp = 0 ;
        do {
            tmp = epoll_wait(epollfd_, events, eventNum, empty() ? 3000 : getTimeout());
            if(!(tmp < 0 && errno == EINTR)) {
                break;
            } 
        } while(true);
        LOG_INFO("epoll_wait tmp = %d fd = %d num = %d errno = %d, %s",tmp,  events[tmp].data.fd, tmp, errno, strerror(errno));
        ASSERT(tmp >= 0, "tmp should not be negative");
       
        // 触发定时任务
        std::vector<std::function<void()>> vec;
        ListTimers(vec);
        for(auto f : vec) {
            pool_.enqueue(f);
        }
        if(tmp == 0) {
            LOG_INFO("timeout with no task");
        }
 
        // epollevent任务响应
        for(int i = 0; i < tmp; ++i) {
            int fd = events[i].data.fd;
            LOG_DEBUG("fd = %d", fd);
            if(fd == pipefd[0]) { 
                LOG_INFO("tick");
                uint8_t dummy[256];
                while(read(fd, dummy, sizeof(dummy)) > 0);
                continue; 
            }
            LOG_INFO("fd = %d has task", fd);
            Fd* fd_ = fds_[fd]; 

            int real_event = NONE;
            if(events[i].events & EPOLLIN) {
                real_event |= EPOLLIN;
            }
            if(events[i].events & EPOLLOUT) {
                real_event |= EPOLLOUT;
            }

            // 删除读写事件
            int op = (~real_event & events[i].events) ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            struct epoll_event eve;
            eve.data.fd = fd;
            eve.events = EPOLLET | (events[i].events & ~real_event);
            epoll_ctl(epollfd_, op, fd, &eve);

            if(EPOLLIN | events[i].events) {
                LOG_INFO("fd= %d trigger event READ", fd);
                fd_->triggerEvent(READ);
            } else if(EPOLLOUT | events[i].events) {
                LOG_INFO("fd= %d trigger event WRITE", fd);
                fd_->triggerEvent(WRITE);
            }
        }
    }
}

void IOManager::AddEvent(int fd, EventType type, std::function<void()>  cb) {
    LOG_INFO("fa = %d, type= %d", fd, type);
    Fd* fd_ = getFd(fd);

    int op = fd_->events_ ?   EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    struct epoll_event event;
    event.events = EPOLLET | op | fd_->events_;
    event.data.fd = fd;
    int ret = epoll_ctl(epollfd_, op, fd, &event);
    if(ret != 0) {
        ASSERT(ret == 0, "epoll_ctl fail");
    }
    Fd::Event& ctx = fd_->getEvent(type);
    if(cb) {
        ctx.cb.swap(cb);
    }
}
void IOManager::DelEvent(int fd, EventType type) {
    if(fd > fds_.size()) { return ; }

    Fd* fd_ = getFd(fd);
    if(!fd_) { return ; }

    epoll_event events;
    events.events = (EventType)(fd_->events_ & ~type);
    int op = events.events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

    epoll_ctl(epollfd_, op, fd, &events);
}


Fd* IOManager::getFd(int fd) {
    if(fds_.size() <= fd) {
        SetFdsSize(fds_.size() * 2);
    }
    return fds_[fd];
}
void IOManager::tick() {
//     printf("tick %ld ---- %ld\n", &pipefd[0], &pipefd[1]);
    int num = write(pipefd[1], "T", 1);
    // LOG_INFO("num = %d", num);
    // ASSERT(num == 1, "tick write failure");
}
}