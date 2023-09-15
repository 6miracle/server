#include "Address/Address.h"
#include "Http/Http.h"
#include "Http/HttpParser.h"
#include "Http/HttpSession.h"
#include "Socket/Socket.h"
#include "URI/URI.h"
#include <memory>
#include <thread>

const char test_request_data[] = "POST / HTTP/1.1\r\n"
                                "Host: www.sylar.top\r\n"
                                "Content-Length: 10\r\n\r\n"
                                "1234567890";

void test_request() {
    server::HttpRequestParser parser;
    std::string tmp = test_request_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    std::cout  << "execute rt=" << s << '\n'
        << "has_error=" << parser.hasError()<< '\n'
        << "is_finished=" << parser.isFinished()<< '\n'
        << "total=" << tmp.size()<< '\n'
        << "content_length=" << parser.getContentLength()<< '\n';
    tmp.resize(tmp.size() - s);
    std::cout << parser.data()->ToString() << '\n';
    std::cout << tmp << '\n';
}

const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
        "Date: Tue, 04 Jun 2019 15:43:56 GMT\r\n"
        "Server: Apache\r\n"
        "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
        "ETag: \"51-47cf7e6ee8400\"\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: 81\r\n"
        "Cache-Control: max-age=86400\r\n"
        "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
        "Connection: Close\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html>\r\n"
        "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
        "</html>\r\n";

void test_response() {
    server::HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t s = parser.execute(&tmp[0], tmp.size(), true);
    std::cout << "execute rt=" << s
        << " has_error=" << parser.hasError()
        << " is_finished=" << parser.isFinished()
        << " total=" << tmp.size()
        << " content_length=" << parser.getContentLength()
        << " tmp[s]=" << tmp[s];

    tmp.resize(tmp.size() - s);

   std::cout << parser.data()->ToString() << '\n'; 
   std::cout << tmp << '\n';
}

void test_http_session() {
    server::Address::ptr addr = server::Address::LookupAny("localhost");
    std::dynamic_pointer_cast<server::IPv4Address>(addr)->setPort(8000);
    server::Socket::ptr sock = server::Socket::CreateTCPSocket();
    sock->bind(addr);
    sock->listen();
    sock->accept();
    // while(1) {
    server::HttpSession session(sock, false);
    std::cout << "request = " << session.RecvRequest()->ToString() << "\n";
    // }
    server::HttpResponse::ptr response = std::make_shared<server::HttpResponse>();
    std::cout << "response = " << session.SendResponse(response) << '\n';
    
}

void test_uri() {
    server::Uri::ptr uri = server::Uri::Create("http://admin@www.sylar.top/test/中文/uri?id=100&name=sylar&vv=中文#frg中文");
    std::cout << uri->ToString() << '\n';
    
}
void thread_test(){
    std::cout << std::this_thread::get_id() << '\n';
}
int main() {
    // test_request();
    // std::cout << "========================================================\n";
    // test_response();
    thread_test();
    // test_http_session();
    // test_uri();
}
