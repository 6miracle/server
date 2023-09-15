#pragma once
#include "Stream.h"
#include "Address/Address.h"
#include "Socket/Socket.h"
namespace server {
class SockStream: public Stream {
public:
    using ptr = std::shared_ptr<SockStream>;

    SockStream(Socket::ptr sock, bool owner = false);
    ~SockStream();

    int read(void* buffer, size_t length) override;
    int read(ByteArray::ptr ba, size_t length) override;

    int write(void* buffer, size_t length) override;
    int write(ByteArray::ptr ba, size_t length) override;

    bool close() override; 
    bool isConnected() const;

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();
    std::string getRemoteAddressString();
    std::string getLocalAddressString();

    Socket::ptr GetSock() {  return sock_; }
private:
    Socket::ptr sock_;
    bool owner_;
};
}