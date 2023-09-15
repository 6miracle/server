#include "HttpSession.h"
#include "Http/Http.h"
#include "Http/HttpParser.h"
#include "Stream/SockStream.h"
#include <sstream>

namespace server {
HttpSession::HttpSession(Socket::ptr sock, bool owner): SockStream(sock, owner) {}

HttpRequest::ptr HttpSession::RecvRequest() {
    HttpRequestParser::ptr parser = std::make_shared<HttpRequestParser>();
    uint64_t buffer_size = HttpRequestParser::GetHttpRequestBufferSize();
    char* buffer = new char[buffer_size];
    int offset = 0;
    do {
        int ret = read(buffer + offset, buffer_size - offset);
        if(ret <= 0) {close();  delete[] buffer; return nullptr;}
        ret += offset;
        buffer[ret] = '\0';
        size_t nParse = parser->execute(buffer, ret);
        if(parser->hasError()) { close();   delete[] buffer; return nullptr; }
        offset = ret - nParse;
        if(offset == buffer_size) { close(); delete[] buffer; return nullptr; }
        if(parser->isFinished()) break;
    } while(true);

    uint64_t length = parser->getContentLength();
    if(length > 0) {
        std::string body(buffer, offset);
        parser->data()->setBody(body);
    }
    return parser->data();
}
int HttpSession::SendResponse(HttpResponse::ptr response) {
    std::cout<< " send response" << '\n';
    std::string s = response->ToString();
    std::cout << s << '\n';
    return writeFixSize((void*)s.c_str(), s.size());
}
}