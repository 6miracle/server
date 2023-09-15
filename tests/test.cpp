#include "Address/Address.h"
#include "IOManager/IOManager.h"
#include "Logger/logger.hpp"
#include "Socket/Socket.h"
#include "Stream/SockStream.h"
#include "TcpServer/TcpServer.h"
#include "config/config.h"
#include "ByteArray/ByteArray.h"

// #include "Thread/ThreadPool.hpp"
// #include <arpa/inet.h>
// #include <cstddef>
// #include <exception>
// #include <memory>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <unistd.h>
// void configTest() {
//     server::ConfigVar<int>::ptr age = server::Config::LookUp("name.age", 13, "saADFAS");
//     server::Config config;
//     config.LoadFromYaml("../config/config.yaml");
//     // config.TurnIntoYaml("../config/yy.yaml");
//     std::cout << age->getVal() << '\n';
// }
void addressTest() {
     std::vector<server::Address::ptr> vecs;
    server::Address::Lookup(vecs, "www.baidu.com:80");

    for(size_t i = 0; i < vecs.size(); ++i) {
        std::cout << vecs[i]->ToString() << '\n';
    }
    
}

// void IOTest1() {
//     LOG("IOTest1");
//     // server::Socket::ptr sock = server::Socket::CreateTCPSocket();
   
//     // std::string host = "localhost";
//     // server::Address::ptr address = server::Address::LookupAny(host);
//     // // if(address == nullptr) { std::cout <<" ==============" <<'\n'; return ; }
//     // // LOG("333");
//     // std::dynamic_pointer_cast<server::IPv4Address>(address)->setPort(8000);
//     // // LOG("========");
//     // sock->connect(address);
//     int sock = socket(AF_INET, SOCK_STREAM, 0);
//     if(sock == 0) {
//         sock = socket(AF_INET, SOCK_STREAM, 0);
//     }
//     struct sockaddr_in address;
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = inet_addr("127.0.0.1");
//     address.sin_port = htons(8000);
//     int error = connect(sock, (struct sockaddr*)&address.sin_addr, sizeof(address));
//     if(error < 0) {
//         std::cout << "IOTest:  connection error" << '\n';
//     }
//        // std::cout << 333 << std::endl;
//     server::IOManager::getInstance()->AddEvent(sock, server::READ, []() {
//         while(1) {
//           std::cout << "\e[0;32m READ \e[0m" <<'\n';
//         }
//     });
//     //  LOG("444");
//     server::IOManager::getInstance()->AddEvent(sock, server::WRITE, []() {
//         while(1) {
//          std::cout << "WRITE" << '\n';
//         }
//     });
//     // while(1) {

//     // LOG("333");
//     // }
//     //  LOG("555");
// }
// void IOTest2() {
//     server::Socket::ptr sock = server::Socket::CreateTCPSocket();
//     std::string host;
//     server::Address::ptr address = server::Address::LookupAny(host);
//     sock->bind(address);
//     sock->listen();
    
//     while(1) {
//         server::Socket::ptr client = sock->accept();
//         char buf[] = "heloo";
//         client->send(buf, sizeof(buf));
//     }
// }
// void IoTest() {
//     server::IOManager* handle = server::IOManager::getInstance();
//     // handle->enqueue(IOTest1);
//     // handle->enqueue([]() { std::cout << "=====" <<'\n';});
// }
// void sockTest() {
//     server::Socket::ptr sock = server::Socket::CreateTCPSocket();
//     server::Address::ptr addr = server::Address::LookupAny("localhost");
//     std::dynamic_pointer_cast<server::IPv4Address>(addr)->setPort(8000);
//     // sock->connect(addr);
//     struct sockaddr_in* addr1 = (sockaddr_in*)(addr->getAddr());
//     sock->newSock();
//     sock->connect(addr, 1000);
//     // char buf[129]= "1111";
//     // int size =  sock->recv(buf, sizeof(buf) - 1);
//     // buf[size - 1] = '\0';
//     // std::cout << buf << '\n';
//     char buf[125]= "1111";
//     std::cout << sock->send(buf, 4) << '\n';

// }

// void byteArrayTest() {
//     server::ByteArray array;
//     // array.writeStringF16("6");
//     // std::cout <<"str = " << array.readStringF16() << '\n';
//     array.writeFuint64(123);
//     // array.writeFuint16(6);
//     // std::cout << "res = " << (int)array.readFuint16() << '\n';
//     std::cout << "res = " << array.readFuint64() << '\n';
// }

// void sockStreamTest() {

//     // server::Address::ptr address = server::Address::LookupAny("localhost");
//     // std::dynamic_pointer_cast<server::IPv4Address>(address)->setPort(8000);
//     // server::Socket::ptr sock = server::Socket::CreateTCPSocket();
//     // sock->connect(address);

//     // char buf[1024] = {0};
//     // while(1) {
//     //     int size = sock->recv(buf, sizeof(buf) - 1);
//     //     if(size > 0) {
//     //         buf[size] = '\0';
//     //         std::cout << "buf = " << buf << '\n';
//     //     }
//     // }
//     server::Address::ptr address = server::Address::LookupAny("localhost");
//     std::dynamic_pointer_cast<server::IPv4Address>(address)->setPort(8000);
//     server::Socket::ptr sock = server::Socket::CreateTCPSocket();
//     sock->connect(address);
//     server::SockStream sockstream(sock);
//      char buf[1024] = {0};
//     int size = sockstream.read(buf, sizeof(buf) - 1);
//     buf[size] = '\0';
//     std::cout << "buf = " << buf <<'\n';
// }

// void tcpserverTest() {
//     server::TcpServer::ptr server = std::make_shared<server::TcpServer>();
//     server::Address::ptr address = server::Address::LookupAny("localhost");
//     std::dynamic_pointer_cast<server::IPv4Address>(address)->setPort(8000);
//     server->bind(address);
//     server->start();
// }
int main() {
    addressTest();
    return 0;
}