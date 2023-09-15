// #include <arpa/inet.h>
// #include <fcntl.h>
// #include <iostream>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <cstring>
// #include <unistd.h>
// #include <thread>

// void func(int* fd) {
//     char buf[1024] = {0};
//     size_t size = read(*fd, buf, sizeof(buf));
//     buf[size] = '\0';
//     std::cout << buf <<'\n';
// }
// int main() {
//     int pipefd[2];
//     pipe(pipefd);

//     char buf[1024];
//     std::thread th(func, &pipefd[0]);
//     write(pipefd[1], "ss", 2);
//       th.join();
//     // int size = read(pipefd[0], buf, sizeof(buf));
//     // buf[size] = '\0';
//     // std::cout << buf << '\n';
// }