#pragma once
#include "Http/Http.h"
#include "Socket/Socket.h"
#include "Stream/SockStream.h"
#include "pch.h"
#include "URI/URI.h"
#include <chrono>
#include <list>
#include <mutex>
#include<atomic>
// 客户端
namespace server {
class HttpConnection:public SockStream {
public:
    using ptr = std::shared_ptr<HttpConnection>;

    HttpConnection(Socket::ptr sock, bool owner = true);

    HttpResponse::ptr recvResponse();
    int sendRequest(HttpRequest::ptr request);

    static HttpResponse::ptr DoGet(const std::string& url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
    static HttpResponse::ptr DoGet(const Uri::ptr url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
    static HttpResponse::ptr DoPost(const std::string& url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
    static HttpResponse::ptr DoPost(const Uri::ptr url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
    static HttpResponse::ptr DoRequest(HttpMethod method, const std::string& url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
    static HttpResponse::ptr DoRequest(HttpMethod method, const Uri::ptr url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
    static HttpResponse::ptr DoRequest(HttpRequest::ptr request, const Uri::ptr uri, uint64_t timeout);

    std::chrono::steady_clock::time_point GetCreateTime() { return createTime_; }
    uint64_t GetRequest() { return request_; }
    void addRequest() { request_++; }
private:
   std::chrono::steady_clock::time_point  createTime_;
   uint64_t request_;

};




class HttpConnectionPool {
public:
        using ptr = std::shared_ptr<HttpConnectionPool>;

        HttpConnectionPool(const std::string& host, uint32_t port, uint32_t max_size, uint32_t maxAliveTime, uint32_t maxRequest);
        HttpConnection::ptr GetConnection();

        
        HttpResponse::ptr DoGet(const std::string& url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
        HttpResponse::ptr DoGet(const Uri::ptr url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
        HttpResponse::ptr DoPost(const std::string& url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
        HttpResponse::ptr DoPost(const Uri::ptr url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
        HttpResponse::ptr DoRequest(HttpMethod method, const std::string& url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
        HttpResponse::ptr DoRequest(HttpMethod method, const Uri::ptr url, uint64_t timeout, 
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "");
        HttpResponse::ptr DoRequest(HttpRequest::ptr request, uint64_t timeout);

        bool isValid(HttpConnection* conn);
private:
        static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);
        std::string host_;
        std::string vhost_;
        uint32_t port_;
        uint32_t maxSize_;
        uint32_t maxAliveTime_;
        uint32_t maxRequest_;
        std::list<HttpConnection* > lists;
        std::mutex mutex_;
        std::atomic<int> total_ = {0};
};
}