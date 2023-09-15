#include "Http.h"
#include <ctime>
#include <sstream>

namespace server {

// 枚举值与字符串转换
const char* HttpMethodStr(HttpMethod method) {
    switch(method) {
#define XX(num, name, str) case HTTP_##name: return #str;
        HTTP_METHOD_MAP(XX)
#undef XX
    return "<unknown>";
    }
    return "<unknown>";
}
const char* HttpStatusStr(HttpStatus status) {
    switch(status) {
#define XX(num, name, str) case HTTP_STATUS_##name: return #str;
        HTTP_STATUS_MAP(XX)
#undef XX
    default:
        return "<unknown>";
    }
}

HttpMethod CharsToMethod(const char* s) {
#define XX(num, name, str) if(!strncmp(#str, s, strlen(#str))) { return  HTTP_##name; }
    HTTP_METHOD_MAP(XX)
#undef XX
    return INVALID_METHOD;
}

HttpMethod StrToMethod(const std::string& s) {
#define XX(num, name, str) if(!strcmp(#str, s.c_str())) { return  HTTP_##name; }
    HTTP_METHOD_MAP(XX)
#undef XX
    return INVALID_METHOD;
}

HttpStatus StrToStatus(const std::string& s) {
#define XX(num, name, str) if(!strcmp(#str, s.c_str())) { return  HTTP_STATUS_##name; }
    HTTP_STATUS_MAP(XX)
#undef XX
    return INVALID_STATUS;
}


HttpRequest::HttpRequest(uint8_t version, bool close): version_(version), close_(close), path_("/"),flag_(0) {}


bool HttpRequest::getHeaderParam(const std::string& key, std::string& value) const {
    auto it = headers_.find(key);
    if(it == headers_.end()) { return false; }
    value = it->second;
    return true;
}
bool HttpRequest::getParam(const std::string& key,std::string& value) const {
    auto it = params_.find(key);
    if(it == params_.end()) { return false; }
    value = it->second;
    return true;
}
bool HttpRequest::getCookie(const std::string& key, std::string& value) {
    initCookies();
    auto it = cookies_.find(key);
    if(it == cookies_.end()) { return false; }
    value = it->second;
    return true;
}

void HttpRequest::setHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
}
void HttpRequest::setParam(const std::string& key, const std::string& value) {
    params_[key] = value;
}
void HttpRequest::setCookie(const std::string& key, const std::string& value) {
    cookies_[key] = value;
}

void HttpRequest::delHeader(const std::string& key) {
    auto it = headers_.find(key);
    if(it != headers_.end()) headers_.erase(key);
}
void HttpRequest::delParam(const std::string& key) {
    auto it = params_.find(key);
    if(it != params_.end()) { params_.erase(key); }
}
void HttpRequest::delCookie(const std::string& key) {
    auto it = cookies_.find(key);
    if(it != cookies_.end()) { cookies_.erase(key); }
}

std::string HttpRequest::ToString() const {
    std::stringstream ss;
    ss << HttpMethodStr(method_) << " "
        <<path_  
        << (query_.empty() ? "" : "?" )
        << query_
        << (fragment_.empty() ? "" : "#")
        << fragment_
        << " HTTP/" << ((uint32_t)(version_ >> 4)) << "." << ((uint32_t)(version_ & 0x0F)) << "\r\n";
  
    for(auto i : headers_) {
        ss << i.first << ": "<<i.second << "\r\n";
    }
    ss << "connection: " << (close_ ? "close" : "keep-alive") << "\r\n";
    if(!body_.empty()) {
        ss << "content-length: " << body_.size() << "\r\n";
    }
    ss << "\r\n";
    ss << body_;
    return ss.str();
}

void HttpRequest::initParam() {
    initBodyParam();
    initQueryParam();
    initBodyParam();
}
void HttpRequest::initQueryParam() {
    size_t pos = 0;
    do{
        size_t r1 = query_.find("=", pos);
        if(r1 == std::string::npos) {
            break;
        }
        size_t r2 = query_.find("&", r1);
        if(r2 == std::string::npos) r2 = query_.size();
        setParam(query_.substr(pos, r1 - pos), query_.substr(r1 + 1, r2 - r1 - 1));
        pos = r2 + 1;
    } while(true);
}
void HttpRequest::initBodyParam() {
    std::string content_type;
    getHeaderParam("content-type", content_type);
    if(strcasestr(content_type.c_str(), "application/x-www-form-urlencoded") == nullptr) {
        return;
    }
    size_t pos = 0;
    do{
        size_t r1 = body_.find("=", pos);
        if(r1 == std::string::npos) {
            break;
        }
        size_t r2 = body_.find("&", r1);
        if(r2 == std::string::npos) r2 = query_.size();
        setParam(body_.substr(pos, r1 - pos), body_.substr(r1 + 1, r2 - r1 - 1));
        pos = r2 + 1;
    } while(true);
}
void HttpRequest::initCookies() {
    std::string cookie;
    if(!getHeaderParam("Cookie", cookie)) { return ; }
    size_t pos = 0;
    do{
        size_t r1 = cookie.find("=", pos);
        if(r1 == std::string::npos) {
            break;
        }
        size_t r2 = cookie.find(";", r1);
        if(r2 == std::string::npos) r2 = query_.size();
        setCookie(body_.substr(pos, r1 - pos), body_.substr(r1 + 1, r2 - r1 - 1));
        pos = r2 + 1;
    } while(true);
}

//========================Response===================

HttpResponse::HttpResponse(uint8_t version, bool close)
    : status_(HTTP_STATUS_OK), version_(version), close_(close){}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
}
bool HttpResponse::getHeaderParam(const std::string& key, std::string& value) const {
    auto it = headers_.find(key);
    if(it == headers_.end()) {
        return false;
    }
    value = it->second;
    return true;
}

void HttpResponse::delHeader(const std::string& key) {
    auto it = headers_.find(key);
    if(it != headers_.end()) { headers_.erase(it); }
}

std::string HttpResponse::ToString() const {
    std::stringstream ss;
    ss << "HTTP/" << ((uint32_t)(version_ >> 4)) << "." << ((uint32_t)(version_ & 0x0F)) << " "
        << (uint32_t)status_ << " "
        << (reason_.empty() ? HttpStatusStr(status_) : reason_)
         << "\r\n";
  
    for(auto i : headers_) {
        ss << i.first << ": "<<i.second << "\r\n";
    }
    ss << "connection: " << (close_ ? "close" : "keep-alive") << "\r\n";
    
    for(auto i : cookies) {
        ss << "Set-cookies: " << i <<"\r\n";
    }
     if(!body_.empty()) {
        ss << "content-length: " << body_.size() << "\r\n";
    }
    ss << "\r\n";
    ss << body_;
    
    return ss.str();
}

void HttpResponse::setRedirect(const std::string& uri) {
    status_ = HTTP_STATUS_FOUND;
    setHeader("Location: ", uri);
}
void HttpResponse::setCookie(const std::string& key, const std::string& val,
                time_t expired, const std::string& path,
                const std::string& domain, bool secure) 
{   
    std::stringstream ss;
    ss << key << "=" << val;
    if(expired >= 0) {
        struct tm* t = localtime(&expired);
        char buf[128] = {0};
        strftime(buf, 128, "%a, %d %b %Y %H:%M:%S", t);
        ss <<";Expires=" << buf << " GMT";
    }
    if(!domain.empty()) {
        ss << ";Domain="<<domain;
    }
    if(!path.empty()) {
        ss <<";Path=" << path;
    }
    if(secure) {
        ss <<";Secure";
    }
    cookies.emplace_back(ss.str());
}
}
