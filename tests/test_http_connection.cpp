#include "HttpConnection/HttpConnection.h"
#include "IOManager/IOManager.h"

void test_pool() {
    server::HttpConnectionPool::ptr pool(new server::HttpConnectionPool(
                "www.baidu.com",  80, 10, 1000 * 30, 5));
//  server::IOManager::GetThis()->addTimer(1000, [pool](){
    auto r = pool->DoGet("/", 300);
    std::cout << r->ToString() << "\n";
    // }, true);
    std::cout << "==============================" << '\n';
}

void run() {
    server::Address::ptr addr = server::Address::LookupAnyIPAddress("www.server.top:80");
    if(!addr) {
        std::cout<< "get addr error" <<'\n';
        return;
    }

    server::Socket::ptr sock = server::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt) {
        std::cout << "connect " << addr->ToString() << " failed" << '\n';
        return;
    }

    server::HttpConnection::ptr conn(new server::HttpConnection(sock));
    server::HttpRequest::ptr req(new server::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host", "www.server.top");
    // std::cout << "req:" << std::endl
    //     << *req << '\n';

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if(!rsp) {
       std::cout << "recv response error" <<'\n';
        return;
    }
    std::cout << "rsp:" << std::endl
        << *rsp << '\n';

//     std::ofstream ofs("rsp.dat");
//     ofs << *rsp;

//    std::cout << "=========================" << '\n';

//     auto r = server::HttpConnection::DoGet("http://www.server.top/blog/", 300);
//     // std::cout << "result=" << r->result
//     //     << " error=" << r->error
//     //     << " rsp=" << (r->response ? r->response->toString() : "");
//     std::cout << r->ToString() << '\n';
//     std::cout << "=========================" << '\n';
//     test_pool();
}

void test_https() {
    auto r = server::HttpConnection::DoGet("http://www.baidu.com/", 300, {
                        {"Accept-Encoding", "gzip, deflate, br"},
                        {"Connection", "keep-alive"},
                        {"User-Agent", "curl/7.29.0"}
            });
    std::cout << r->ToString() << '\n';
    server::HttpConnectionPool::ptr pool(new server::HttpConnectionPool(
               "www.baidu.com",  80,  10, 1000 * 30, 5));
    // server::HttpConnectionPool pool = server::HttpConnectionPool(
    //                 "https://www.baidu.com", "", 10, 1000 * 30, 5);
    server::IOManager::getInstance()->AddTimer(1000, [pool](){
            auto r = pool->DoGet("/", 3000, {
                        {"Accept-Encoding", "gzip, deflate, br"},
                        {"User-Agent", "curl/7.29.0"}
                    });
           std::cout  << r->ToString() << '\n';
    }, true);
}

void test_data() {
    server::Address::ptr addr = server::Address::LookupAnyIPAddress("www.baidu.com:80");
    auto sock = server::Socket::CreateTCP(addr);

    sock->connect(addr);
    const char buff[] = "GET / HTTP/1.1\r\n"
                "connection: close\r\n"
                "Accept-Encoding: gzip, deflate, br\r\n"
                "Host: www.baidu.com\r\n\r\n";
    sock->send(buff, sizeof(buff));

    std::string line;
    line.resize(1024);

    std::ofstream ofs("http.dat", std::ios::binary);
    int total = 0;
    int len = 0;
    while((len = sock->recv(&line[0], line.size())) > 0) {
        total += len;
        ofs.write(line.c_str(), len);
    }
    std::cout << "total: " << total << " tellp=" << ofs.tellp() << std::endl;
    ofs.flush();
}

// void test_parser() {
//     std::ifstream ifs("http.dat", std::ios::binary);
//     std::string content;
//     std::string line;
//     line.resize(1024);

//     int total = 0;
//     while(!ifs.eof()) {
//         ifs.read(&line[0], line.size());
//         content.append(&line[0], ifs.gcount());
//         total += ifs.gcount();
//     }

//     std::cout << "length: " << content.size() << " total: " << total << std::endl;
//     server::http::HttpResponseParser parser;
//     size_t nparse = parser.execute(&content[0], content.size(), false);
//     std::cout << "finish: " << parser.isFinished() << std::endl;
//     content.resize(content.size() - nparse);
//     std::cout << "rsp: " << *parser.getData() << std::endl;

//     auto& client_parser = parser.getParser();
//     std::string body;
//     int cl = 0;
//     do {
//         size_t nparse = parser.execute(&content[0], content.size(), true);
//         std::cout << "content_len: " << client_parser.content_len
//                   << " left: " << content.size()
//                   << std::endl;
//         cl += client_parser.content_len;
//         content.resize(content.size() - nparse);
//         body.append(content.c_str(), client_parser.content_len);
//         content = content.substr(client_parser.content_len + 2);
//     } while(!client_parser.chunks_done);

//     std::cout << "total: " << body.size() << " content:" << cl << std::endl;

//     server::ZlibStream::ptr stream = server::ZlibStream::CreateGzip(false);
//     stream->write(body.c_str(), body.size());
//     stream->flush();

//     body = stream->getResult();

//     std::ofstream ofs("http.txt");
//     ofs << body;
// }

int main(int argc, char** argv) {
    // server::IOManager iom(2);
    // //iom.schedule(run);
    // iom.schedule(test_https);
    // 
    // std::cout << server::Address::LookupAnyIPAddress("www.baidu.com:80")->ToString() << '\n';
    // server::IOManager::getInstance()->enqueue(run);
    test_data();
    // test_https();
    return 0;
}
