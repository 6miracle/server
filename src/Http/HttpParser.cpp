#include "HttpParser.h"
#include "Http/Http.h"
#include "HttpParser/http11_parser.h"
#include "HttpParser/httpclient_parser.h"
#include "Logger/logger.hpp"
#include <cstring>
#include <string>

namespace server {

void on_request_method(void *data, const char *at, size_t length) {
    HttpRequestParser* ptr = static_cast<HttpRequestParser*>(data);
    HttpMethod method = CharsToMethod(at);

    if(method == INVALID_METHOD) {
        LOG_ERROR("Method Error");
        ptr->setError_(1);
        return ;
    }
    // ptr->data()->setStatus(StrToStatus(HttpMethodStr(method)));
    ptr->data()->setMethod(method);
}
void on_request_uri(void *data, const char *at, size_t length) {

}
void on_fragment(void *data, const char *at, size_t length) {
    HttpRequestParser* ptr = static_cast<HttpRequestParser*>(data);
    ptr->data()->setFragment(std::string(at, length));
}
void on_request_path(void *data, const char *at, size_t length) {
    HttpRequestParser* ptr = static_cast<HttpRequestParser*>(data);
    ptr->data()->setPath(std::string(at, length));
}
void on_query_string(void *data, const char *at, size_t length) {
    HttpRequestParser* ptr = static_cast<HttpRequestParser*>(data);
    ptr->data()->setQuery(std::string(at, length));
}

void on_http_version(void *data, const char *at, size_t length) {
    HttpRequestParser* ptr = static_cast<HttpRequestParser*>(data);
    uint8_t v = 0;
    if(strncmp(at, "HTTP/1.1", length) == 0) v = 0x11;
    else if(strncmp(at, "HTTP/1.0", length)) v = 0x10;
    else {
        LOG_ERROR("INVAILD http version");
        return ;
    }
    ptr->data()->setVersion(v);
}
void on_header_done(void *data, const char *at, size_t length) {

}

void on_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen) {
    HttpRequestParser* ptr = static_cast<HttpRequestParser*>(data);
    if(flen == 0) {
        LOG_ERROR(" invalid request field");
        return ;
    }
    ptr->data()->setHeader(std::string(field, flen), std::string(value, vlen));
}

HttpRequestParser::HttpRequestParser(): error_(0){ 
    request_ = std::make_shared<HttpRequest>();
    http_parser_init(&parser_);
    parser_.request_method = on_request_method;
    parser_.request_uri = on_request_uri;
    parser_.fragment = on_fragment;
    parser_.request_path = on_request_path;
    parser_.query_string = on_query_string;
    parser_.http_version = on_http_version;
    parser_.header_done = on_header_done;
    parser_.http_field = on_http_field;
    parser_.data = this;
}


bool HttpRequestParser::isFinished()  {
    return ::http_parser_is_finished(&parser_);
}
bool HttpRequestParser::hasError() {
    return error_ || http_parser_has_error(&parser_);
}
size_t HttpRequestParser::execute(char* data, size_t len) {
    size_t offset = http_parser_execute(&parser_, data, len, 0);
    memmove(data, data + offset, len - offset);
    return offset;
}
uint64_t HttpRequestParser::getContentLength() {
    std::string value;
    request_->getHeaderParam("content-length", value);
    if(value.empty()) { return 0; }
    return std::stoull(value);
}


// ======================== Response==============
void on_response_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen) {
    HttpResponseParser* parser_ = static_cast<HttpResponseParser*>(data);
    if(flen == 0) {
        LOG_ERROR("INVALID resonse http field");
        return ;
    }
    parser_->data()->setHeader(std::string(field, flen), std::string(value, vlen));
}

void on_response_reason_phrese(void *data, const char *at, size_t length) {
    HttpResponseParser* parser_ = static_cast<HttpResponseParser*>(data);
    parser_->data()->setReason(std::string(at, length));
}
void on_response_status_code(void *data, const char *at, size_t length) {
    HttpResponseParser* parser_ = static_cast<HttpResponseParser*>(data);
    HttpStatus status = (HttpStatus)atoi(at);
    parser_->data()->setStatus(status);
}
void on_response_chunk_size(void *data, const char *at, size_t length) {

}
void on_response_http_version(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    uint8_t v = 0;
    if(strncmp(at, "HTTP/1.1", length) == 0) v = 0x11;
    else if(strncmp(at, "HTTP/1.0", length)) v = 0x10;
    else {
        LOG_ERROR("INVAILD http version");
        return ;
    }
    parser->data()->setVersion(v);
}

void on_response_header_done(void *data, const char *at, size_t length) {

}
void on_response_last_chunk(void *data, const char *at, size_t length) {

}

HttpResponseParser::HttpResponseParser(): error_(0) {
    response_ = std::make_shared<HttpResponse>();
    httpclient_parser_init(&parser_);
    parser_.header_done = on_response_header_done;
    parser_.last_chunk = on_response_last_chunk;
    parser_.http_version = on_response_http_version;
    parser_.http_field = on_response_http_field;
    parser_.reason_phrase = on_response_reason_phrese;
    parser_.chunk_size = on_response_chunk_size;
    parser_.status_code = on_response_status_code;
    parser_.data = this;
}

bool HttpResponseParser::isFinished() {
    return httpclient_parser_is_finished(&parser_);
}
bool HttpResponseParser::hasError()  {
    return error_ || httpclient_parser_has_error(&parser_);
}
size_t HttpResponseParser::execute(char* data, size_t len, bool chunk) {
    if(chunk) {
        httpclient_parser_init(&parser_);
    }
    size_t offset = httpclient_parser_execute(&parser_, data, len, 0);
    memmove(data, data + offset, (len - offset));
    return offset;
}

uint64_t HttpResponseParser::getContentLength() {
    std::string value;
    response_->getHeaderParam("content-length", value);
    if(value.empty()) {  return 0;}
    return std::stoull(value);
}


}