#include "TcpServer.h"
#include "Address/Address.h"
#include "IOManager/IOManager.h"
#include "Socket/Socket.h"
#include <netinet/in.h>
#include <utility>


namespace server {
TcpServer::TcpServer(): timeout_(60 * 1000 * 2), name("server/1.0.0"), stop_(true) {
    worker_ = IOManager::getInstance();
}
TcpServer::~TcpServer() {
    for(auto i : socks_) {

        i->close();
    }
    socks_.clear();
}

bool TcpServer::bind(Address::ptr addr) {
    std::vector<Address::ptr> address{addr};
    std::vector<Address::ptr> fails;
    return bind(address, fails);
}
bool TcpServer::bind(const std::vector<Address::ptr> addr, std::vector<Address::ptr> fails) {
    if(addr.size() == 0) { return true; }
    for(Address::ptr address : addr) {
        Socket::ptr sock = Socket::CreateTCPSocket();
        if(!sock->bind(address)) {
            LOG_ERROR("SOCK  = %dbind error", sock->getSock());
            fails.push_back(address);
            continue;
        }
        if(!sock->listen()) {
            LOG_ERROR("sock%d listen error", sock->getSock());
            fails.push_back(address);
            continue;
        }
        LOG_INFO("SOCK %d listen address %s", sock->getSock(),address->ToString().c_str());
        socks_.push_back(std::move(sock));
    }
    if(!fails.empty()) {
        socks_.clear();
        return false;
     }
    return fails.empty();
    
}

bool TcpServer::start() {
    stop_ = false;

    for(Socket::ptr sock : socks_) {
        // worker_->enqueue(std::bind(&TcpServer::startAccept, this, sock));
        LOG_DEBUG("sock %d is being ", sock->getSock());
        worker_->AddEvent(sock->getSock(), EventType::READ, std::bind(&TcpServer::startAccept, this, sock));
    }
    worker_->loop();
    return true;
}
bool TcpServer::stop() {
    stop_ = true;
    worker_->enqueue([this]() {
        for(auto sock : socks_) {
            sock->cancelAll();
            sock->close();
        }
        socks_.clear();
    });
    return true;
}

std::string TcpServer::ToString() const {
    return "TcpServer";
}
void TcpServer::handleClient(Socket::ptr client) {
    LOG_DEBUG("tcpserver handle client");
 }

void TcpServer::startAccept(Socket::ptr sock) {
    while(!stop_) {
        Socket::ptr client = sock->accept();
        if(!client) {
            // LOG_ERROR("sock accept error, sock ");
            continue;
        } else {
            LOG_INFO("accept client %d", client->getSock());
        }
        client->setRecvTimeout(timeout_);
        // worker_->enqueue(std::bind(&TcpServer::handleClient, this, client));
        worker_->AddEvent(client->getSock(), READ, std::bind(&TcpServer::handleClient, this, client));
    }
} 
}