#include "Address.h"
#include <arpa/inet.h>
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>

namespace server {
template<typename T>
static T CreateMask(T prefix) {
    return (1 << (sizeof(T) * 8 - prefix)) - 1;
}
bool Address::Lookup(std::vector<Address::ptr>& result, const std::string& host,
            int family, int type, int protocol) 
{
    addrinfo hint, *results;
    memset(&hint, 0, sizeof(hint));

    hint.ai_flags = 0;
    hint.ai_family = family;
    hint.ai_socktype = type;
    hint.ai_protocol = protocol;
    hint.ai_addrlen = 0;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;

    char* service;
    std::string node;
    if(!host.empty() && host[0] == '[') {
        service = (char*)memchr(host.c_str(), ']', host.size());
        if(service) {
            node = host.substr(1, service - host.c_str() - 1);
            if(*(service + 1) == ':') {
                service += 2;
            }
        }   
    } else {
        service = (char*)memchr(host.c_str(), ':', host.size());
        if(service) {
            node = host.substr(0, service - host.c_str());
            service += 1;
        } else {
            service = NULL;
        }
    }

    if(node.empty()) node = host;
    int err = 0;
    do {
        int err = getaddrinfo(node.c_str(), service, &hint, &results);
    } while(err == -3);
    // if(err!=0) { std::cout << node.c_str() <<" " << service << "err = " <<err << '\n'; }
    ASSERT(err == 0, "getaddrinfo Error");
   
    for(addrinfo* rp = results; rp != NULL;rp = rp->ai_next) {
        result.emplace_back(Address::Create(rp->ai_addr, rp->ai_addrlen));
    }

    freeaddrinfo(results);
    return !result.empty();
}   

Address::ptr Address::LookupAny(const std::string& host,int family, int type, int protocol) {
    std::vector<Address::ptr> results;
    Lookup(results,host, family, type, protocol);
    return results.empty() ? nullptr : results[0];
}

 std::shared_ptr<IPAddress> Address::LookupAnyIPAddress(const std::string& host,int family , int type, int protocol) {
    std::vector<Address::ptr> results;
    Address::Lookup(results,host, family, type, protocol);
    return results.empty() ? nullptr : std::dynamic_pointer_cast<IPAddress>(results[0]);
 }

 
bool GetInterfaceAddresses(std::multimap<std::string,std::pair<Address::ptr, uint32_t> >& result,int family = AF_INET) {
    return false;
}

bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result,const std::string& iface, int family = AF_INET) {
    return false;
}
Address::ptr Address::Create(const sockaddr *addr, socklen_t len) {
    if(addr == nullptr) {
        return nullptr;
    }
    //  std::cout << "create " << std::hex << inet_ntoa(((sockaddr_in*)addr)->sin_addr) << '\n' << std::dec;
    Address::ptr result;
    switch(addr->sa_family) {
        case AF_INET:
            result.reset(new IPv4Address(*((struct sockaddr_in*)addr)));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*((struct sockaddr_in6*)addr)));
            break;
        case AF_UNIX:
            result.reset(new UnixAddress(*((struct sockaddr_un*)addr)));
            break;
        default:
            return nullptr;
    }
    return result;
}

IPAddress::ptr IPAddress::Create(const char* addr, uint16_t port) {
    addrinfo hint, *result;
    memset(&hint, 0, sizeof(hint));
    hint.ai_flags = AI_NUMERICHOST;
    hint.ai_family = AF_UNSPEC;

    int err = getaddrinfo(addr, NULL, &hint, &result);
    ASSERT(err ==0, "GetAddrinfo Error");
    
    IPAddress::ptr ptr = std::dynamic_pointer_cast<IPAddress>(Address::Create(result->ai_addr, result->ai_addrlen));
    ptr->setPort(port);

    freeaddrinfo(result);
    return ptr;
}   

IPv4Address::ptr IPv4Address::Create(const char* addr, uint16_t port) {
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET,addr, &address.sin_addr.s_addr);
    return std::make_shared<IPv4Address>(address);
}

IPv4Address::IPv4Address() {
    addr_.sin_addr.s_addr = 0;
    addr_.sin_port = 0;
    addr_.sin_family = AF_INET;
}
IPv4Address::IPv4Address(sockaddr_in& addr):  addr_(addr){
    // std::cout << std::hex << "-----" << inet_ntoa(addr.sin_addr )<< '\n';
    addr_.sin_port = addr.sin_port;
    addr_.sin_family = AF_INET;
}
IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
    addr_.sin_port = htons(port);
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(address);
}

    // 广播地址
IPAddress::ptr IPv4Address::BroadastAddress(uint32_t prefix) const {
    if(prefix > 32) return nullptr;
    sockaddr_in addr(addr_);
    addr.sin_addr.s_addr |= htonl(CreateMask<uint32_t>(prefix));
    IPv4Address::ptr ptr = std::make_shared<IPv4Address>(addr);
    return ptr;
}
// 子网地址
IPAddress::ptr IPv4Address::NetworkAddress(uint32_t prefix) const {
    if(prefix > 32) return nullptr;
    sockaddr_in addr(addr_);
    addr.sin_addr.s_addr &= htonl(~CreateMask<uint32_t>(prefix));
    return std::make_shared<IPv4Address>(addr);
}
// 子网掩码
IPAddress::ptr IPv4Address::SubnetMask(uint32_t prefix) const {
    if(prefix > 32) return nullptr;
    sockaddr_in addr(addr_);
    addr.sin_addr.s_addr =  htonl(~CreateMask<uint32_t>(prefix));
    return std::make_shared<IPv4Address>(addr);
}
uint16_t IPv4Address::getPort() const {
    return ntohs(addr_.sin_port);
}
void IPv4Address::setPort(uint16_t port){
    addr_.sin_port = htons(port);
}

// Address
sockaddr* IPv4Address::getAddr() const {
    return (struct sockaddr*)&addr_;
}
int IPv4Address::getFamily() const {return AF_INET;}
socklen_t IPv4Address::getAddrLen() const {
    return sizeof(addr_);
}

const std::string IPv4Address::ToString() const  {
    std::stringstream ss;
    ss << "IPv4Address " 
        << "address= " << std::hex << inet_ntoa(addr_.sin_addr) << std::dec
        << " port = " << ntohs(addr_.sin_port);
    return ss.str();
}
// ========IPv6===================
IPv6Address::ptr IPv6Address::Create(const char* addr, uint16_t port) {
    if(addr == nullptr) {  return nullptr;}
    sockaddr_in6 address;
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(port);
    inet_pton(AF_INET6, addr, address.sin6_addr.s6_addr);
    return std::make_shared<IPv6Address>(address);
}
IPv6Address::IPv6Address() {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_family = AF_INET6;
}
IPv6Address::IPv6Address(sockaddr_in6& addr):addr_(addr) {}
IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_port = htons(port);
    addr_.sin6_family = AF_INET6;
    memcpy(&addr_.sin6_addr.s6_addr, address, 16);
}

// IP
// 广播地址
IPAddress::ptr IPv6Address::BroadastAddress(uint32_t prefix) const  {
    return nullptr;
}
// 子网地址
IPAddress::ptr IPv6Address::NetworkAddress(uint32_t prefix) const {
    return nullptr;
}
// 子网掩码
IPAddress::ptr IPv6Address::SubnetMask(uint32_t prefix) const {
    return nullptr;
}
uint16_t IPv6Address::getPort() const {
    return ntohs(addr_.sin6_port);
}
void IPv6Address::setPort(uint16_t port) {
    addr_.sin6_port = htons(port);
}

sockaddr* IPv6Address::getAddr() const { return  (sockaddr*)&addr_;}
int IPv6Address::getFamily() const { return AF_INET6; }
socklen_t IPv6Address::getAddrLen() const { return sizeof(addr_); }

const std::string IPv6Address::ToString() const{ 
    std::stringstream ss;
    ss << "IPv4Address " 
        << "address=";
    for(int i = 0 ; i < 8; ++i) {
        uint16_t num = 0;
        num = addr_.sin6_addr.s6_addr[2 * i] * 16 + addr_.sin6_addr.s6_addr[2 * i + 1];
        if(num != 0) {
            ss << std::hex << num;
        }
        if(i < 7) ss << ":";
    }
    ss << "port = " << ntohs(addr_.sin6_port);
    return ss.str();
}

UnixAddress::UnixAddress() {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sun_family = AF_UNIX;
}
UnixAddress::UnixAddress(sockaddr_un& addr): addr_(addr) {}
UnixAddress::UnixAddress(const std::string& path) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sun_family = AF_UNIX;
    strncpy(addr_.sun_path, path.c_str(), sizeof(addr_.sun_path) - 1);
}
    
sockaddr* UnixAddress::getAddr() const { return (sockaddr*)&addr_;}
int UnixAddress::getFamily() const { return AF_UNIX;}
socklen_t UnixAddress::getAddrLen() const { return sizeof(addr_); }
const std::string UnixAddress::ToString() const {
    std::stringstream ss;
    ss << "Unix:" << addr_.sun_path;
     return ss.str();
}

}