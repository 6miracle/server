#pragma once
#include "IOManager/IOManager.h"
#include "Servlet/Servlet.h"
#include "Socket/Socket.h"
#include "TcpServer/TcpServer.h"
#include "Http/HttpSession.h"
#include <memory>

namespace server {
class HttpServer: public TcpServer {
public:
    using ptr = std::shared_ptr<HttpServer>;
    HttpServer(bool keepAlive = false);

    ServletDispatch::ptr getServletDispatch() const { return dispatcher_; };
// protected:
    virtual void handleClient(Socket::ptr client);
private:
    bool keepAlive_;
    ServletDispatch::ptr dispatcher_;
};
}