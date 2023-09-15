// 封装 socket的使用

#pragma once 
#include "pch.h"
#include "Address/Address.h"
#include <bits/types/struct_iovec.h>
#include <cstdint>
#include <sys/socket.h>

namespace server {

class Socket: public std::enable_shared_from_this<Socket>{
public:
    using ptr = std::shared_ptr<Socket>;
    Socket(int domain, int type, int protocol);
    ~Socket();
    Socket(const Socket&) = delete;
    Socket& operator = (const Socket& ) = delete;

     //创建TCP Socket(满足地址类型
    static Socket::ptr CreateTCP(Address::ptr address);

    // brief 创建UDP Socket(满足地址类型)
    static Socket::ptr CreateUDP(Address::ptr address);

     // brief 创建IPv4的TCP Socket
    static Socket::ptr CreateTCPSocket();

    //  创建IPv4的UDP Socket
    static Socket::ptr CreateUDPSocket();

    // 创建IPv6的TCP Socket
    static Socket::ptr CreateTCPSocket6();

    // 创建IPv6的UDP Socket
    static Socket::ptr CreateUDPSocket6();

    // 创建Unix的TCP Socket
    static Socket::ptr CreateUnixTCPSocket();

    // 创建Unix的UDP Socket
    static Socket::ptr CreateUnixUDPSocket();


    // 设置超时时间
    uint64_t getSendTimeout();
    bool setSendTimeout(uint64_t timeout);
    uint64_t getRecvTimeout();
    bool setRecvTimeout(uint64_t timeout);

    // socket option设置
    template <typename T>
    bool setOption(int level, int option, const T& val) {
        return setOption(level, option, &val, sizeof(T));
    }
    bool setOption(int level, int option, const void* val, size_t len);

    bool getOption(int level, int option, void* val, size_t* len);

    // 常规函数
    // 服务端
    bool bind(const Address::ptr addr);
    bool listen(int backlog = SOMAXCONN);
    Socket::ptr accept();

    // 客户端
    bool connect(const Address::ptr addr, uint64_t timeout = -1);
    bool connect_with_timeout(const Address::ptr addr, uint64_t timeout = -1);
    Address::ptr getLocalAddress();
    Address::ptr getRemoteAddress();

    // 通用
    bool close();
    bool newSock();
    bool init(int sock);
    void init();
    bool IsValid() const ;

    int send(const void* buf, size_t len, int flags = 0);
    int send(iovec* buf, size_t len, int flags = 0);
    int recv(iovec* buf, size_t len, int flags = 0);
    int recv( void *buf, size_t len, int flags = 0);

    // 取消读
    bool cancelRead();
    // 取消写
    bool cancelWrite();
    // 取消accept
    bool cancelAccept();
    // 取消所有事件
    bool cancelAll();


    int getSock() const { return sock_; } 
    bool IsConnected() const { return isconnected_; }
    void setNonblock();

    bool noPeer() const {
        if(!peer_ || peer_->getSock() == -1) { return true; }
        return false;
    }
private:
    int sock_;  // socket句柄
    Socket::ptr peer_; //
    int family_;  // 协议族
    int type_;  // 类型
    int protocol_; // 协议
    bool isconnected_; //是否connect
    bool blocked;

    Address::ptr localAddress_;
    Address::ptr remoteAddress_;
};
}