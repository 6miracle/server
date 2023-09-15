// #include <arpa/inet.h>
// #include <fcntl.h>
// #include <iostream>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <cstring>
// #include <unistd.h>
// #include <thread>

// // int main() {
// //     struct addrinfo hints, *res;
// //     // 设置hints
// //     memset(&hints, 0, sizeof hints);
// //     hints.ai_family = AF_INET;
// //     // 域名转地址 
// //     int val = getaddrinfo("www.baidu.com", NULL, &hints, &res);
// //     for(addrinfo* rp = res; rp != NULL;rp = rp->ai_next) { 
// //         std::cout << std::hex << inet_ntoa(((sockaddr_in*)(rp->ai_addr))->sin_addr) << '\n';
// //         std::cout << ((sockaddr_in*)(rp->ai_addr))->sin_port <<'\n';
// //     }
// // }
// static void setNonblock(int fd) {
//     int old = fcntl(fd, F_GETFL);
//     int new_ = old | O_NONBLOCK;
//     fcntl(fd, F_SETFL, new_);
// }
// int main() {
//     int sock = socket(AF_INET, SOCK_STREAM, 0);

//     struct sockaddr_in addr;
//     addr.sin_addr.s_addr = inet_addr("127.0.0.1");
//     addr.sin_port = ntohs(8000);
//     addr.sin_family = AF_INET;
//     setNonblock(sock);
//     uint64_t timeout = 1000;
//     int error = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
//     if(error != 0 && errno == EINPROGRESS) {
//         fd_set rfds;
//         FD_ZERO(&rfds);
//         FD_SET(sock, &rfds);
//         struct timeval val{(int)(timeout / 1000), (int)(timeout% 1000 * 1000)};
//         int ret = select(sock + 1, &rfds,NULL, NULL, &val);
//         if(ret <= 0) {
//             std::cout << "select fail" << '\n';
//             return 0;
//         }
//      }
//     // int err;
//     // socklen_t len = sizeof(err);

//     // getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len);

//     // if (error != 0) {
//     //    std::cout << " connection error"<< '\n';
//     // }  else {
//     //     sleep(1);
//     // }
//     char buf[128] = "1111";
//     int llen = 4;
//     // size_t length = recv(sock, buf, sizeof(buf) - 1,0);
//     int size = 0;
//     int length = 0;
//     while(llen > 0 &&(size = send(sock, buf + length, llen, 0)) >= 0) {
//         // std::cout << "-----------\n";
//         if(size < 0) {
//             if (errno == EAGAIN)
//             {
//                 std::cout << "========" << '\n';
//                 continue;
//             } 
//             exit(-1);
//         } else if(size == 0) {
//             std::cout << " read over" << '\n';
//             break;
//         }
        
//         length += size;
//         llen -= size;
//     }
//     // buf[length] = '\0';
//     // if(length == 0) { std::cout << "recv fail" << '\n'; }
//     // std::cout << "len =" << length << "buf = " <<  buf << '\n';

// }

// int main() {

// }