#include "Socket.h"
#include "Address/Address.h"
// #include "IOManager/IOManager.h"
#include "Logger/logger.hpp"
#include "common.h"
// #include "Stream/SockStream.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>

namespace server {
void Socket::setNonblock() {
    if(sock_ == -1) {
        LOG_INFO("sock should not be -1");
    }
    int old = fcntl(sock_, F_GETFL);
    int new_ = old | O_NONBLOCK;
    fcntl(sock_, F_SETFL, new_);
    blocked =false;
}

Socket::Socket(int domain, int type, int protocol): sock_(-1), family_(domain), 
                type_(type), protocol_(protocol), isconnected_(false) , peer_(), blocked(true){
        
}

Socket::~Socket() {
    if(sock_ != -1) {
        close();
    }   
}

//创建TCP Socket(满足地址类型
Socket::ptr Socket::CreateTCP(Address::ptr address) {
    return std::make_shared<Socket>(address->getFamily(), SOCK_STREAM, 0);
}

// brief 创建UDP Socket(满足地址类型)
Socket::ptr Socket::CreateUDP(Address::ptr address) {
    return std::make_shared<Socket>(address->getFamily(), SOCK_DGRAM, 0);
}

    // brief 创建IPv4的TCP Socket
Socket::ptr Socket::CreateTCPSocket() {
    return std::make_shared<Socket>(AF_INET, SOCK_STREAM, 0);
}

//  创建IPv4的UDP Socket
Socket::ptr Socket::CreateUDPSocket() {
    return std::make_shared<Socket>(AF_INET, SOCK_DGRAM, 0);
}

// 创建IPv6的TCP Socket
Socket::ptr Socket::CreateTCPSocket6() {
    return std::make_shared<Socket>(AF_INET6, SOCK_STREAM, 0);
}

// 创建IPv6的UDP Socket
Socket::ptr Socket::CreateUDPSocket6() {
    return std::make_shared<Socket>(AF_INET6, SOCK_DGRAM, 0);
}

// 创建Unix的TCP Socket
Socket::ptr Socket::CreateUnixTCPSocket() {
    return std::make_shared<Socket>(AF_UNIX, SOCK_STREAM, 0);
}

// 创建Unix的UDP Socket
Socket::ptr Socket::CreateUnixUDPSocket() {
    return std::make_shared<Socket>(AF_UNIX, SOCK_DGRAM, 0);
}
bool Socket::close() {
    LOG_ERROR("sock %d is closed", sock_);
    if(!isconnected_ && sock_ == -1) { return true; }
    isconnected_ = false; 
    if(peer_) peer_->close();
    if(sock_ != -1) {
       int err =  ::close(sock_);
       if(err != 0) { LOG_ERROR("close error"); return false;}
        sock_ = -1;
    }
    return true;
}

uint64_t Socket::getSendTimeout() {
    struct timeval val;
    size_t len = sizeof(val);
    bool r = getOption(SOL_SOCKET, SO_SNDTIMEO, &val, &len);
    return r ? val.tv_sec * 1000 + val.tv_usec / 1000 : -1;
}
bool Socket::setSendTimeout(uint64_t timeout) {
    struct timeval val{(int)(timeout / 1000), (int)(timeout% 1000 * 1000)};
    return setOption(SOL_SOCKET, SO_SNDTIMEO, &val, sizeof(val));
}
uint64_t Socket::getRecvTimeout() {
    struct timeval val;
    size_t len = sizeof(val);
    bool r =  getOption(SOL_SOCKET, SO_RCVTIMEO, &val, &len);
    return r ? val.tv_sec * 1000 + val.tv_usec / 1000 : -1;
}
bool Socket::setRecvTimeout(uint64_t timeout) {
    struct timeval val{(int)(timeout / 1000), (int)(timeout% 1000 * 1000)};
    return  setOption(SOL_SOCKET, SO_RCVTIMEO, &val, sizeof(val));
}


// socket option设置
bool Socket::setOption(int level, int option, const void* val, size_t len) {
    int tmp = setsockopt(sock_, level, option, val, (socklen_t)len);
    if(tmp == -1) {
        LOG_ERROR("setOption sock=%d, level=%d, option=%d, errno=%d, errstr = %s ", \
            sock_, level, option, errno, strerror(errno));
        return false;
    }
    return true;

}
bool Socket::getOption(int level, int option, void* val, size_t* len) {
    int tmp = getsockopt(sock_, level, option, val, (socklen_t*)len);
        if(tmp == -1) {
        LOG_ERROR("getOption sock=%d, level=%d, option=%d, errno=%d, errstr = %s ", \
            sock_, level, option, errno, strerror(errno));
        return false;
    }
    return true;
}

bool Socket::bind(const Address::ptr addr) {
    if(sock_ == -1) {
        newSock();
        if(!IsValid()) { return false; }
    }
    int err = ::bind(sock_, addr->getAddr(), addr->getAddrLen());
    if(err != 0) {
        LOG_ERROR("bind Error %s", strerror(errno));
    }
    // ASSERT(err ==0 , "bind error");
    return err == 0;
}
bool Socket::listen(int backlog) {
    int err = ::listen(sock_, backlog);
    if(err != 0) {
        LOG_ERROR("listen Error");
    }
    return err == 0;
}
Socket::ptr Socket::accept() {
    int client = ::accept(sock_, nullptr, nullptr);
    if(client == -1) {
        // LOG_ERROR("accept error sock= %d, %s", sock_, strerror(errno));
        return nullptr;
    }
    // Socket::ptr sock
    peer_ = std::make_shared<Socket>(family_, type_, protocol_);
    peer_->init(client);
    // peer_ = client;
    isconnected_ = true;
    return peer_;   
}

bool Socket::connect_with_timeout(const Address::ptr addr, uint64_t timeout) {
     int err = ::connect(sock_, addr->getAddr(), addr->getAddrLen());
     if(err == 0) { return true; }
     else if(errno == EINPROGRESS) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sock_, &rfds);
        struct timeval val{(int)(timeout / 1000), (int)(timeout% 1000 * 1000)};
        int ret = select(sock_ + 1, &rfds, NULL, NULL, &val);
        if(ret > 0) {
            return true;
        }
     }
     return false;

} 
bool Socket::connect(const Address::ptr addr, uint64_t timeout) {
    if(sock_ == -1) {
        newSock();
        if(!IsValid()) { return false; }
    }
    if(timeout == (uint64_t)(-1)) {
        std::cout <<"connect " << "addr = " << inet_ntoa(((struct sockaddr_in*)addr->getAddr())->sin_addr) 
                << " port = " << ntohs(((struct sockaddr_in*)addr->getAddr())->sin_port) 
                 << " sock= " <<sock_ 
                 << " len = " << addr->getAddrLen()
                  << '\n';
        int err = ::connect(sock_, addr->getAddr(), addr->getAddrLen());
        if(err != 0) {
            LOG_ERROR("Connection Error %s", strerror(errno));
            close();
            return false;
        }
    } else {
        if(!connect_with_timeout(addr, timeout)) {
            LOG_ERROR("Connection with timeout Error");
            close();
            return false;
        }
    }
    remoteAddress_ = addr;
    isconnected_ = true;
    getLocalAddress();
    return true;
}

int Socket::send(const void* buf, size_t len, int flags) {
    if(isconnected_) {
        if(!blocked) {
            int length = 0;
            int size = 0;
            while((size = ::send(noPeer() ? sock_ : peer_->getSock(), (char*)buf + length, len, flags)) >= 0) {
                if(size < 0) {
                    if (errno == EAGAIN) {
                        continue;
                    } 
                    LOG_ERROR("send fail");
                    return 0;
                } else if(size == 0) {
                    break;
                }
                length += size;
            }
            return length;
        } else {
            return ::send( noPeer()? sock_ : peer_->getSock(), buf, len, flags);
        }
    }
    LOG_INFO("sock is not connected");
    return 0;
}

int Socket::send(iovec* buf, size_t len, int flags) {
    if(!isconnected_) { return -1; }
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = buf;
    msg.msg_iovlen = len;
    return ::sendmsg(noPeer() ? sock_ : peer_->getSock() , &msg, flags);
}
int Socket::recv(iovec* buf, size_t len, int flags) {
    if(!isconnected_) { return -1; }
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = buf;
    msg.msg_iovlen = len;
    return ::recvmsg(noPeer() ? sock_ : peer_->getSock() , &msg, flags);
}

int Socket::recv(void *buf, size_t len, int flags) {
    if(isconnected_) {
        if(!blocked) {
            LOG_INFO("sock is nonblock");
            int length = 0;
            int size = 0;
            while((size = ::recv(noPeer() ? sock_ : peer_->getSock(), (char*)buf + length, len, flags)) >= 0) {
                if(size < 0) {
                    if (errno == EAGAIN)
                    {
                        continue;
                    } 
                    LOG_ERROR("recv fail");
                    return 0;
                } else if(size == 0) {
                    break;
                }
                length += size;
            }
            return length;
        } else {
            return ::recv(noPeer() ? sock_ : peer_->getSock(), buf, len, flags);
        }
    }
    LOG_INFO("sock is not connected");
    return 0;
}
bool Socket::init(int sock) {
    sock_ = sock;
    isconnected_ = true;
    getLocalAddress();
    getRemoteAddress();
    return true;
}
void Socket::init() {
    LOG_DEBUG("sock %d is init", sock_);
    int val = 1; // 1 一般表示启用选项
    setOption(SOL_SOCKET, SO_REUSEADDR,&val); // 允许重用本地地址和端口,不需要timewait
    if(type_ == SOCK_STREAM) {   // 如果是tcp
        setOption(IPPROTO_TCP, TCP_NODELAY, val); //如果是TCP socket,再启用TCP_NODELAY选项,禁用Nagle算法,数据发送更实时
    }   
    setNonblock();
}

bool Socket::IsValid() const {
    return sock_ != -1;
}
bool Socket::newSock() {
    // std::cout << family_ << " " << type_ << " " << protocol_ << '\n'; 
    sock_ = ::socket(family_, type_, protocol_);
    if(sock_ == 0) { 
        LOG_ERROR("sock error");
        sock_ = ::socket(family_, type_, protocol_); 
    }
    LOG_INFO("new sock = %d family = %d, type = %d, protocol = %d", sock_, family_, type_, protocol_);
    if(sock_ != -1) {
        init();
        return true;
    }
    return false;
}
Address::ptr Socket::getLocalAddress() {
    if(localAddress_) {  return localAddress_; }
    if(sock_ == -1) { return nullptr; }
    Address::ptr result;
    switch(family_) {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
    }
    socklen_t len = result->getAddrLen();
    if(::getsockname(sock_, result->getAddr(), &len)) { return nullptr; }
    localAddress_ = result;
    return localAddress_;
}
Address::ptr Socket::getRemoteAddress() {
    if(remoteAddress_) { return remoteAddress_; }
     if(sock_ == -1) { return nullptr; }
    Address::ptr result;
    switch(family_) {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
    }
    socklen_t len = result->getAddrLen();
    if(::getpeername(sock_, result->getAddr(), &len)) { return nullptr; }
    remoteAddress_ = result;
    return localAddress_;

}

// 取消读
bool Socket::cancelRead() {
    return false;
}

// 取消写
bool Socket::cancelWrite() {
    return false;
}

// 取消accept
bool Socket::cancelAccept() {
    return false;
}

// 取消所有事件
bool Socket::cancelAll() {
    return false;
}
}