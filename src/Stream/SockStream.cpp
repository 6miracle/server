#include "SockStream.h"
#include "Socket/Socket.h"
#include <cstddef>

namespace server {
SockStream::SockStream(Socket::ptr sock, bool owner):sock_(sock), owner_(owner) {}

SockStream::~SockStream() { 
    if(owner_) {
        close();
    }
}


int SockStream::read(void* buffer, size_t length) {
    if(!isConnected()) { return -1; }
    return sock_->recv(buffer, length);
}
int SockStream::read(ByteArray::ptr ba, size_t length) {
    if(!isConnected()) { return -1; }
    std::vector<iovec> iovs;
    ba->getWriteBuffers(iovs, length);
    int rt = sock_->recv(&iovs[0], iovs.size());
    if(rt > 0) {
        ba->setPosition(ba->getPosition() + rt);
    }
    return rt;
}

int SockStream::write(void* buffer, size_t length) {
    if(!isConnected()) { return -1; }
    return sock_->send(buffer, length);
}
int SockStream::write(ByteArray::ptr ba, size_t length) {
    if(!isConnected()) { return -1; }
    std::vector<iovec> iovs;
    ba->getReadBuffers(iovs, length);
    int rt = sock_->send(&iovs[0], iovs.size());
    if(rt > 0) {
        ba->setPosition(ba->getPosition() + rt);
    }
    return rt;
}

bool SockStream::close()  {
    //  LOG_INFO("======================= %s", CURRENT_THREAD().c_str());
    if(owner_) {
        return sock_->close();
    }
    return true;
}
bool SockStream::isConnected() const {
    return sock_ && sock_->IsConnected();
}

Address::ptr SockStream::getRemoteAddress() {
    return sock_->getRemoteAddress();
}
Address::ptr SockStream::getLocalAddress() {
    return sock_->getLocalAddress();
}
std::string SockStream::getRemoteAddressString() {
    return sock_->getRemoteAddress()->ToString();
}
std::string SockStream::getLocalAddressString() {
    return sock_->getLocalAddress()->ToString();
}
}