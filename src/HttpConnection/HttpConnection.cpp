#include "HttpConnection.h"
#include "Address/Address.h"
#include "Http/Http.h"
#include "Http/HttpParser.h"
#include "Socket/Socket.h"
#include "Stream/SockStream.h"
#include "URI/URI.h"
#include <chrono>
#include <cstddef>
#include <memory>
#include <mutex>
#include <sstream>
#include <strings.h>

namespace server {
static std::chrono::steady_clock::time_point GetCurrentMs() {
    return std::chrono::steady_clock::now();
}


HttpConnection::HttpConnection(Socket::ptr sock, bool owner): SockStream(sock, owner) {}
HttpResponse::ptr HttpConnection::recvResponse() {
     HttpResponseParser::ptr parser = std::make_shared<HttpResponseParser>();
    uint64_t buffer_size = HttpResponseParser::GetHttpRequestBufferSize();
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
    std::string body;
    if(parser->IsChunked()) {
        while(!parser->IsChunkedDone()) {
                bool begin = true;
                do {
                        if(!begin || offset == 0) {
                        int ret = read(buffer + offset, buffer_size - offset);
                        if(ret <= 0) {close();  delete[] buffer; return nullptr;}
                        offset += ret;
                        }
                        buffer[offset] = '\0';
                        size_t nParse = parser->execute(buffer, offset, true);
                        if(parser->hasError()) { close();   delete[] buffer; return nullptr; }
                        offset -= nParse;
                        if(offset == buffer_size) { close(); delete[] buffer; return nullptr; }
                        begin = false;
                } while(!parser->isFinished());
                if(parser->getContentLen() + 2 <= offset) {
                        body.append(buffer, parser->getContentLen());
                        memmove(buffer, buffer + parser->getContentLen() + 2, 
                                offset - parser->getContentLen() - 2);
                        offset -= parser->getContentLen() + 2;
                } else {
                        body.append(buffer, offset);
                        int left = parser->getContentLen() - offset + 2;
                         while(left > 0) {
                                int rt = read(buffer, left > (int)buffer_size ? (int)buffer_size : left);
                                if(rt <= 0) {
                                        close();
                                        return nullptr;
                                }
                                body.append(buffer, rt);
                                left -= rt;
                        }
                        body.resize(body.size() - 2);
                        offset = 0;
                        }

        }

    } else {
        uint64_t length = parser->getContentLength();
        if(length > 0) {
                std::string body(buffer, offset);
                parser->data()->setBody(body);
        }
    }
    return parser->data();

}
int HttpConnection::sendRequest(HttpRequest::ptr request) {
    std::stringstream ss;
    ss << request->ToString();
    std::string s = ss.str();
    return writeFixSize(s.c_str(),s.size());
}

HttpResponse::ptr HttpConnection::DoGet(const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers,
        const std::string& body ) 
{
       Uri::ptr uri =  Uri::Create(url);
       return DoGet(uri, timeout, headers, body);
}
HttpResponse::ptr HttpConnection::DoGet(const Uri::ptr url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers,
        const std::string& body) 
{
        return DoRequest(HTTP_GET, url, timeout, headers, body);
}
HttpResponse::ptr HttpConnection::DoPost(const std::string& url, uint64_t timeout, 
    const std::map<std::string, std::string>& headers,
    const std::string& body)
{
        Uri::ptr uri =  Uri::Create(url);
       return DoPost(uri, timeout, headers, body);
}
HttpResponse::ptr HttpConnection::DoPost(const Uri::ptr url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers ,
        const std::string& body) 
{
        return DoRequest(HTTP_POST, url, timeout, headers, body);
}
HttpResponse::ptr HttpConnection::DoRequest(HttpMethod method, const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers,
        const std::string& body) 
{
        Uri::ptr uri =  Uri::Create(url);
       return DoRequest(method, uri, timeout, headers, body);
}
HttpResponse::ptr HttpConnection::DoRequest(HttpMethod method, const Uri::ptr url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers,
        const std::string& body)
{
        HttpRequest::ptr request = std::make_shared<HttpRequest>();
        request->setMethod(method);
        request->setPath(url->getPath());
        request->setQuery(url->getQuery());
        request->setFragment(url->getFragment());
        bool flag = false;
        for(auto& p : headers) {
             if(strcasecmp(p.first.c_str(), "connection") == 0 && 
                strcasecmp(p.second.c_str(), "keep-alive") == 0 ) {
                        request->setClose(false);
                }   
                if(!flag && strcasecmp("Host", p.first.c_str()) == 0) {
                        flag = !p.second.empty();
                }
                request->setHeader(p.first, p.second);
        }
        if(!flag) {
                request->setHeader("Host", url->getHost());
        }

        request->setBody(body);
        return DoRequest(request, url, timeout);
}

HttpResponse::ptr HttpConnection::DoRequest(HttpRequest::ptr request, const Uri::ptr uri, uint64_t timeout) {
        Address::ptr addr = uri->CreateAddress();
        Socket::ptr sock = Socket::CreateTCP(addr);

        sock->connect(addr);
        sock->setRecvTimeout(timeout);

        HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);

        int ret = conn->sendRequest(request);
        if(ret <= 0) {
                LOG_ERROR("http connection send request failure");
                return nullptr;
        }
        HttpResponse::ptr rsp = conn->recvResponse();
        return rsp;
}



HttpConnectionPool::HttpConnectionPool(const std::string& host, uint32_t port, 
        uint32_t max_size, uint32_t maxAliveTime, uint32_t maxRequest):
                host_(host), port_(port), maxSize_(max_size), maxAliveTime_(maxAliveTime), maxRequest_(maxRequest) {
}
 bool HttpConnectionPool::isValid(HttpConnection* conn) {
        return conn && conn->isConnected() && 
        (conn->GetCreateTime() + std::chrono::milliseconds(maxAliveTime_) < GetCurrentMs()
        && (conn->GetRequest() < maxRequest_)); 
 }
HttpConnection::ptr HttpConnectionPool::GetConnection() {
        auto now = GetCurrentMs();
        std::vector<HttpConnection*> invalid;
        HttpConnection* conn = nullptr;

        {
                std::lock_guard<std::mutex> mtx(mutex_);
                while(!lists.empty()) {
                        HttpConnection* c = lists.front();
                        lists.pop_front();
                        if(!isValid(c)) {
                                invalid.push_back(c);
                                continue;
                        }
                        conn = c;
                        break;
                }
        }

        total_ -= invalid.size();
        for(auto l : invalid) delete l;
        if(!conn) {
               IPAddress::ptr addr = Address::LookupAnyIPAddress(host_);
                addr->setPort(port_);
                Socket::ptr sock = Socket::CreateTCP(addr);
                sock->connect(addr);
                conn = new HttpConnection(sock);
                total_++;
        }
        return HttpConnection::ptr(conn, std::bind(&HttpConnectionPool::ReleasePtr, std::placeholders::_1, this));
}
void HttpConnectionPool::ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool) {
        ptr->addRequest();
        if(!pool->isValid(ptr)) {
                delete ptr;
                --pool->total_;
                return ;
        }
        std::lock_guard<std::mutex> mtx_(pool->mutex_);
        pool->lists.push_back(ptr);
}

HttpResponse::ptr HttpConnectionPool::DoGet(const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers,
        const std::string& body) {
                Uri::ptr uri = Uri::Create(url);
                if(!uri) {
                  LOG_ERROR("uri = %s create error", url.c_str());
                }
                LOG_INFO("get uri = %s", uri->ToString().c_str());
               return DoGet(uri, timeout, headers, body);
        }
HttpResponse::ptr HttpConnectionPool::DoGet(const Uri::ptr url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers,
        const std::string& body) {
                return DoRequest(HttpMethod::HTTP_GET, url, timeout, headers, body);
        }
HttpResponse::ptr HttpConnectionPool::DoPost(const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers,
        const std::string& body) {
                return DoPost(Uri::Create(url), timeout, headers, body);
        }
HttpResponse::ptr HttpConnectionPool::DoPost(const Uri::ptr url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers,
        const std::string& body) {
                return DoRequest(HttpMethod::HTTP_POST, url, timeout, headers, body);
        }
HttpResponse::ptr HttpConnectionPool::DoRequest(HttpMethod method, const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers,
        const std::string& body) {
                return DoRequest(method, Uri::Create(url), timeout, headers, body);
        }
HttpResponse::ptr HttpConnectionPool::DoRequest(HttpMethod method, const Uri::ptr url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers,
        const std::string& body) {
              HttpRequest::ptr request = std::make_shared<HttpRequest>();
                request->setMethod(method);
                request->setPath(url->getPath());
                request->setQuery(url->getQuery());
                request->setFragment(url->getFragment());
                bool flag = false;
                for(auto& p : headers) {
                if(strcasecmp(p.first.c_str(), "connection") == 0 && 
                        strcasecmp(p.second.c_str(), "keep-alive") == 0 ) {
                                request->setClose(false);
                        }   
                        if(!flag && strcasecmp("Host", p.first.c_str()) == 0) {
                                flag = !p.second.empty();
                        }
                        request->setHeader(p.first, p.second);
                }
                if(!flag) {
                        request->setHeader("Host", vhost_);
                }

                request->setBody(body);
                return DoRequest(request, timeout);
        }
HttpResponse::ptr HttpConnectionPool::DoRequest(HttpRequest::ptr request,  uint64_t timeout) {
        HttpConnection::ptr conn = GetConnection();
        auto sock = conn->GetSock();
        sock->setRecvTimeout(timeout);
        // std::cout << "============REQUEST+====== \n";
        // std::cout << request->ToString();
        // std::cout << "============REQUEST+====== \n";
        conn->sendRequest(request);
        return conn->recvResponse();
}
}