#include "HttpServer.h"
#include "Http/Http.h"
#include "Http/HttpSession.h"
#include "Logger/logger.hpp"
#include "Servlet/Servlet.h"
#include "TcpServer/TcpServer.h"
#include <sstream>
#include <thread>

namespace server {
HttpServer::HttpServer(bool keepAlive):keepAlive_(keepAlive), dispatcher_(std::make_shared<ServletDispatch>()){
    dispatcher_->setDefault(std::make_shared<NotFoundServlet>());
}
void HttpServer::handleClient(Socket::ptr client) {
    LOG_INFO("handle client %d", client->getSock());
    HttpSession::ptr session = std::make_shared<HttpSession>(client);
    do {
        // LOG_INFO("======================= %s", CURRENT_THREAD().c_str());
        auto request = session->RecvRequest();
        if(request != NULL) {
            LOG_INFO("RECV REQUEST %s", request->ToString().c_str());
        } else {
            LOG_ERROR("bad request");
        }
        HttpResponse::ptr res = std::make_shared<HttpResponse>(request->getVersion(), request->IsClose() || !keepAlive_);
        res->setHeader("Server", "Http");
        dispatcher_->Handle(request, res,session);
        session->SendResponse(res);
        if(!keepAlive_ || request->IsClose()) {
            break;
        }
    }while(true);
    LOG_INFO("client %d ok", client->getSock());
    session->close();
    // std::cout<< "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << '\n';
}
}