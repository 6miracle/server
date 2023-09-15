#pragma once 
#include "pch.h"
#include <cstdint>
#include <sys/socket.h>
#include <sys/un.h>

namespace server {
class IPAddress;
class Address {
public:
    using ptr = std::shared_ptr<Address>;

        /**
     * @brief 通过host地址返回对应条件的所有Address
     * @param[out] result 保存满足条件的Address
     * @param[in] host 域名,服务器名等.举例: www.sylar.top[:80] (方括号为可选内容)
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回是否转换成功
     */
    static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,
            int family = AF_INET, int type = SOCK_STREAM, int protocol = 0);
    /**
     * @brief 通过host地址返回对应条件的任意Address
     * @param[in] host 域名,服务器名等.举例: www.sylar.top[:80] (方括号为可选内容)
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回满足条件的任意Address,失败返回nullptr
     */
    static Address::ptr LookupAny(const std::string& host,
            int family = AF_INET, int type = SOCK_STREAM, int protocol = 0);
    /**
     * @brief 通过host地址返回对应条件的任意IPAddress
     * @param[in] host 域名,服务器名等.举例: www.sylar.top[:80] (方括号为可选内容)
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回满足条件的任意IPAddress,失败返回nullptr
     */
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
            int family = AF_INET, int type = SOCK_STREAM, int protocol = 0);

    // TODO: 完成
    /**
     * @brief 返回本机所有网卡的<网卡名, 地址, 子网掩码位数>
     * @param[out] result 保存本机所有地址
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @return 是否获取成功
     */
    static bool GetInterfaceAddresses(std::multimap<std::string
                    ,std::pair<Address::ptr, uint32_t> >& result,
                    int family = AF_INET);
    /**
     * @brief 获取指定网卡的地址和子网掩码位数
     * @param[out] result 保存指定网卡所有地址
     * @param[in] iface 网卡名称
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @return 是否获取成功
     */
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result
                    ,const std::string& iface, int family = AF_INET);


    virtual ~Address() = default;
    // 创建一个Address
    static Address::ptr Create(const sockaddr* addr, socklen_t len);         


    virtual sockaddr* getAddr() const = 0;
    virtual int getFamily() const = 0;
    virtual socklen_t getAddrLen() const = 0;

    virtual const std::string ToString() const = 0;
};

class IPAddress: public Address {
public:
    using ptr = std::shared_ptr<IPAddress>;

    static IPAddress::ptr Create(const char* addr, uint16_t port = 0);

    // 广播地址
    virtual IPAddress::ptr BroadastAddress(uint32_t prefix) const = 0;
    // 子网地址
    virtual IPAddress::ptr NetworkAddress(uint32_t prefix) const = 0;
    // 子网掩码
    virtual IPAddress::ptr SubnetMask(uint32_t prefix) const = 0;

    virtual uint16_t getPort() const = 0;
    virtual void setPort(uint16_t port) = 0;

};


class IPv4Address: public IPAddress {
public:
    using ptr = std::shared_ptr<IPv4Address>;
    // 点分十进制创建
    static IPv4Address::ptr Create(const char* addr, uint16_t port = 0);

    IPv4Address();
    IPv4Address(struct sockaddr_in& addr);
    // 通过二进制地址构造IPv4Address
    IPv4Address(uint32_t address, uint16_t port = 0);

    // IP
     // 广播地址
    IPAddress::ptr BroadastAddress(uint32_t prefix) const override;
    // 子网地址
    IPAddress::ptr NetworkAddress(uint32_t prefix) const override;
    // 子网掩码
    IPAddress::ptr SubnetMask(uint32_t prefix) const override;
    uint16_t getPort() const override;
    void setPort(uint16_t port) override;

    // Address
    sockaddr* getAddr() const override;
    int getFamily() const override;
    socklen_t getAddrLen() const override;

    const std::string ToString() const override;
private:
    struct sockaddr_in  addr_;
};


class IPv6Address: public IPAddress {
public:
    using ptr = std::shared_ptr<IPv6Address>;

    static IPv6Address::ptr Create(const char* addr, uint16_t port = 0);
    IPv6Address();
    IPv6Address(struct sockaddr_in6& addr);
    IPv6Address(const uint8_t address[16], uint16_t port = 0);

      // IP
     // 广播地址
    IPAddress::ptr BroadastAddress(uint32_t prefix) const override;
    // 子网地址
    IPAddress::ptr NetworkAddress(uint32_t prefix) const override;
    // 子网掩码
    IPAddress::ptr SubnetMask(uint32_t prefix) const override;
    uint16_t getPort() const override;
    void setPort(uint16_t port)override;
    
    sockaddr* getAddr() const override;
    int getFamily() const override;
    socklen_t getAddrLen() const override;

     const std::string ToString() const override;
private:
    struct sockaddr_in6 addr_;
};


class UnixAddress: public Address {
public:
    using ptr = std::shared_ptr<UnixAddress>;

    UnixAddress();
    UnixAddress(sockaddr_un& addr);
    UnixAddress(const std::string& path);
     
    sockaddr* getAddr() const override;
    int getFamily() const override;
    socklen_t getAddrLen() const override;
    const std::string ToString() const override;
private:
    sockaddr_un addr_;
};

class UnknownAddress: public Address {
public:
    const std::string ToString() const { return "Unknown"; }
    sockaddr* getAddr() { return  nullptr;}
    int getFamily(){ return 0; }
    socklen_t getAddrLen() const { return 0; }
};
}