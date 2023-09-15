#pragma once
#include "Address/Address.h"
#include "IOManager/IOManager.h"
#include "Socket/Socket.h"
#include "pch.h"
#include <memory>

namespace server {
class TcpServer: public std::enable_shared_from_this<TcpServer>{
public:
    using ptr = std::shared_ptr<TcpServer>;
        
    TcpServer();
    ~TcpServer();

    virtual bool bind(Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr> addr, std::vector<Address::ptr> fails);

    virtual bool start();
    virtual bool stop();

    virtual std::string ToString() const;
// protected:

    // 处理新连接的客户
    virtual void handleClient(Socket::ptr client);

    //  开始接受连接
    virtual void startAccept(Socket::ptr sock);

private:
    IOManager* worker_; // 线程池管理
    std::vector<Socket::ptr> socks_;
    uint64_t timeout_;
    std::string type= "tcp";
    std::string name;
    bool stop_;
};
}