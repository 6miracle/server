#pragma once
#include "Http/Http.h"
#include "Stream/SockStream.h"
#include "HttpParser.h"
#include <memory>
namespace server {
class HttpSession: public SockStream {
public:
    using ptr = std::shared_ptr<HttpSession>;

    HttpSession(Socket::ptr sock, bool owner = true);

    HttpRequest::ptr RecvRequest();
    int SendResponse(HttpResponse::ptr response);
private:

};
}