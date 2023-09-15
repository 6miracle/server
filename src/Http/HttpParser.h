#pragma once 
#include "pch.h"
#include "Http/Http.h"
#include "HttpParser/http11_parser.h"
#include "HttpParser/httpclient_parser.h"


namespace server {
class HttpRequestParser {
public:
    using ptr = std::shared_ptr<HttpRequestParser>;
    HttpRequestParser();

    bool isFinished() ;
    bool hasError();
    size_t execute(char* data, size_t len);

    void setError_(int error) { error_ = error; }

    HttpRequest::ptr data() { return request_; }

    static uint64_t GetHttpRequestBufferSize() { return 4 * 1024;  }
    uint64_t getContentLength();
private:
    HttpRequest::ptr request_;
    http_parser parser_;
    int error_;
};

class HttpResponseParser {
public:
    using ptr = std::shared_ptr<HttpResponseParser>;
    HttpResponseParser();
    
    bool isFinished() ;
    bool hasError();
    size_t execute(char* data, size_t len, bool chunk = false);

    HttpResponse::ptr data() { return response_; }
    static uint64_t GetHttpRequestBufferSize() { return 4 * 1024;  }
    uint64_t getContentLength();
    bool IsChunked() const { return parser_.chunked; }
    bool IsChunkedDone() const { return parser_.chunks_done; }
    int getContentLen() const  { return parser_.content_len; }
private:
    HttpResponse::ptr response_;
    httpclient_parser parser_;
    int error_;
};
}