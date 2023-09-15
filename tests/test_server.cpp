#include "Address/Address.h"
#include "Http/HttpSession.h"
#include "HttpServer/HttpServer.h"
#include "IOManager/IOManager.h"
#include "Servlet/Servlet.h"
#include "Socket/Socket.h"
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <thread>

#define XX(...) #__VA_ARGS__


server::IOManager::ptr worker;
void run() {
    //server::http::HttpServer::ptr server(new server::http::HttpServer(true, worker.get(), server::IOManager::GetThis()));
    server::HttpServer::ptr server(new server::HttpServer(true));
    server::Address::ptr addr = server::Address::LookupAnyIPAddress("127.0.0.1:8020");
    std::cout << "addr " <<std::hex<<  inet_ntoa(((struct sockaddr_in*)addr->getAddr())->sin_addr) <<std::dec<< '\n';
    // while(!server->bind(addr)) {
    //     sleep(2);
    // }
    server->bind(addr);
    auto sd = server->getServletDispatch();
    sd->addServlet("/server/xx", [](server::HttpRequest::ptr req
                ,server::HttpResponse::ptr rsp
                ,server::HttpSession::ptr session) {
            rsp->setBody(req->ToString());
            return 0;
    });

    sd->addGlobServlet("/server/*", [](server::HttpRequest::ptr req
                ,server::HttpResponse::ptr rsp
                ,server::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->ToString());
            return 0;
    });

    sd->addGlobServlet("/serverx/*", [](server::HttpRequest::ptr req
                ,server::HttpResponse::ptr rsp
                ,server::HttpSession::ptr session) {
            rsp->setBody(XX(<html>
<head><title>404 Not Found</title></head>
<body>
<center><h1>404 Not Found</h1></center>
<hr><center>nginx/1.16.0</center>
</body>
</html>
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
));
            return 0;
    });

    LOG_ERROR("prepared");
    server->start();
}
void setNonblock(int sock_) {
    if(sock_ == -1) {
        LOG_INFO("sock should not be -1");
    }
    int old = fcntl(sock_, F_GETFL);
    int new_ = old | O_NONBLOCK;
    fcntl(sock_, F_SETFL, new_);
}
void server1() {
    //server::http::HttpServer::ptr server(new server::http::HttpServer(true, worker.get(), server::IOManager::GetThis()));
    // server::HttpServer::ptr server(new server::HttpServer(true));
    server::Address::ptr addr = server::Address::LookupAnyIPAddress("0.0.0.0:8020");
    // std::cout << "addr " <<std::hex<<  inet_ntoa(((struct sockaddr_in*)addr->getAddr())->sin_addr) <<std::dec<< '\n';
    // server->bind(addr);
    // server->start();
    server::Socket::ptr sock = server::Socket::CreateTCPSocket();
}

void epollTest() {
    server::Address::ptr address = server::Address::LookupAnyIPAddress("127.0.0.1:8000");
    server::Socket::ptr sock = server::Socket::CreateTCPSocket();
    setNonblock(sock->getSock());
    sock->bind(address);
    sock->listen();

    int epollfd = epoll_create(5);
    struct epoll_event event;
    struct epoll_event events[1024];
    event.data.fd = sock->getSock();
    event.events = EPOLLET | EPOLLIN | EPOLLOUT;
    int op = EPOLL_CTL_ADD;
    int tmp  = epoll_ctl(epollfd, op, sock->getSock(), &event);
    LOG_ERROR("%d-%d-%d", epollfd, op, event.events);
    if(tmp == -1) {
        std::cerr << errno << " str = " << strerror(errno) << '\n';
    } else {
        std::cout << "tmp = " << tmp << '\n';
    }

    int num = epoll_wait(epollfd, events, 1000, -1);

}
void io() {
    server::IOManager* io = server::IOManager::getInstance();
    server::Address::ptr address = server::Address::LookupAnyIPAddress("127.0.0.1:8000");
    server::Socket::ptr sock = server::Socket::CreateTCPSocket();
    setNonblock(sock->getSock());
    sock->bind(address);
    sock->listen();

    io->AddEvent(sock->getSock(), server::READ, [](){std::cout << " ======== " << '\n';});
}

void thread_test(){
    std::cout << std::this_thread::get_id() << '\n';
}

void test_session() {
    server::Address::ptr address = server::Address::LookupAnyIPAddress("127.0.0.1:8000");
    server::Socket::ptr sock = server::Socket::CreateTCPSocket();
    sock->bind(address);
    sock->listen();
    server::Socket::ptr client = sock->accept();
    
    server::HttpSession::ptr session = std::make_shared<server::HttpSession>(client);
    std::cout << session->RecvRequest()->ToString() << '\n';

}

void test_http_server() {
        server::Address::ptr address = server::Address::LookupAnyIPAddress("127.0.0.1:8000");
    server::Socket::ptr sock = server::Socket::CreateTCPSocket();
    sock->bind(address);
    sock->listen();
    server::Socket::ptr client = sock->accept();
    // server::HttpServer::ptr server = std::make_shared<server::HttpServer>();
    // server->handleClient(client);
     LOG_INFO("handle client %d", client->getSock());
    server::HttpSession::ptr session = std::make_shared<server::HttpSession>(client);
    server::ServletDispatch::ptr dispatcher = std::make_shared<server::ServletDispatch>();
    dispatcher->setDefault(std::make_shared<server::NotFoundServlet>());
    do {
        LOG_INFO("======================= %s", CURRENT_THREAD().c_str());
        auto request = session->RecvRequest();
        if(request != NULL) {
            LOG_INFO("RECV REQUEST %s", request->ToString().c_str());
        } else {
            LOG_ERROR("bad request");
        }
        LOG_INFO("!!!!!!!!!!!!!!");
        server::HttpResponse::ptr res = std::make_shared<server::HttpResponse>(request->getVersion(), request->IsClose());
        res->setHeader("Server", "Http");
        dispatcher->Handle(request, res,session);
        session->SendResponse(res);
        if(request->IsClose()) {
            break;
        }
    }while(true);
    LOG_INFO("client %d ok", client->getSock());
    session->close();
}
int main(int argc, char** argv) {
    // server::IOManager iom(1, true, "main");
    // worker.reset(new server::IOManager(3, false, "worker"));
    // iom.schedule(run);
    // epollTest();
    run();
    // test_session();
    // io();
    // thread_test();
    // test_http_server();
    return 0;
}
