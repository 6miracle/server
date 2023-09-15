#pragma once
#include "pch.h"
#include <ostream>

namespace server {

/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \

enum HttpStatus {
#define XX(code, name, str) HTTP_STATUS_##name = code,
    HTTP_STATUS_MAP(XX)
#undef XX
    INVALID_STATUS
};


/* Request Methods */
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)       \

enum HttpMethod {
#define XX(num, name, str) HTTP_##name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
    INVALID_METHOD
};

const char* HttpMethodStr(HttpMethod method);
const char* HttpStatusStr(HttpStatus status);

HttpMethod StrToMethod(const std::string& s);
HttpStatus StrToStatus(const std::string& s);
HttpMethod CharsToMethod(const char* s);
struct CompareStr {
    bool operator()(const std::string& str1, const std::string& str2) const{
        return strcasecmp(str1.c_str(), str2.c_str()) < 0;
    }
};

class HttpRequest {
public:
    using ptr = std::shared_ptr<HttpRequest>;
    using MapType = std::map<std::string, std::string,CompareStr>;
    HttpRequest(uint8_t version = 0x11, bool close = true);
    void help() {
        auto q = [](){return false;};
    }

    HttpMethod getMethod() const noexcept { return method_; }
    HttpStatus getStatus() const {return status_; }
    uint8_t getVersion() const { return version_; }
    const std::string& getBody() { return body_; }
    const MapType& getHeaders() { return headers_; }

    void setVersion(uint8_t version) { version_ = version; }
    void setBody(const std::string& v) { body_ = v; initBodyParam();}
    void setHeaders(const MapType& v) { headers_ = v; }
    void setQuery(const std::string& s) { query_ = s; initQueryParam(); }
    void setPath(const std::string& s) { path_ = s; }
    void setFragment(const std::string& s) { fragment_ = s;}

    void setMethod(HttpMethod v) { method_ = v; }
    void setStatus(HttpStatus v) { status_ = v; }
    void setClose(bool v) { close_ = v; }


    bool getHeaderParam(const std::string& key, std::string& value) const;
    bool getParam(const std::string& key,std::string& value) const ;
    bool getCookie(const std::string& key, std::string& value);

    void setHeader(const std::string& key, const std::string& value);
    void setParam(const std::string& key, const std::string& value);
    void setCookie(const std::string& key, const std::string& value);

    void delHeader(const std::string& key);
    void delParam(const std::string& key);
    void delCookie(const std::string& key);

    void initParam();
    void initQueryParam();
    void initBodyParam();
    void initCookies();
    //TODO
    std::string ToString() const;
    std::string GetPath() const { return path_; }
    bool IsClose() { return close_;}
private:
    HttpMethod method_;
    HttpStatus status_;
    uint8_t version_;
    bool close_;
    int flag_;

    // https://zhuanlan.zhihu.com/p/216488873
    std::string path_;     // 路径
    std::string query_;     // ？后面的参数
    std::string fragment_;  // #后面的

    std::string body_;    // 请求消息体

    MapType headers_;   // 头部参数
    MapType params_;    // 请求参数
    MapType cookies_;   // cookie

};

class HttpResponse {
public:
    using ptr = std::shared_ptr<HttpResponse>;
    using MapType = std::map<std::string, std::string,CompareStr>;
    HttpResponse(uint8_t version = 0x11, bool close = true);

    HttpStatus getStatus() const {return status_; }
    uint8_t getVersion() const { return version_; }
    const std::string& getBody() { return body_; }
    const std::string& getReason() { return reason_; }
    const MapType& getHeaders() { return headers_; }

    void setStatus(HttpStatus v) { status_ = v; }
    void setVersion(uint8_t version) { version_ = version; }
    void setBody(const std::string& v) { body_ = v;}
    void setReason(const std::string& v) { reason_ = v;}
    void setHeaders(const MapType& v) { headers_ = v; }

    void setHeader(const std::string& key, const std::string& value);
    bool getHeaderParam(const std::string& key, std::string& value) const;
    void delHeader(const std::string& key);

    bool isClose() {return  close_;}
    void setClose(bool v) { close_ = v; }

    std::string ToString() const;

    void setRedirect(const std::string& uri);
    void setCookie(const std::string& key, const std::string& val,
                   time_t expired = 0, const std::string& path = "",
                   const std::string& domain = "", bool secure = false);

private:
    HttpStatus status_;
    uint8_t version_;
    bool close_;

    std::string body_;    // 响应消息体
    std::string reason_;
    MapType headers_;   // 头部参数

    std::vector<std::string> cookies;
};

inline std::ostream& operator<<(std::ostream& os, const HttpRequest request) {
    os << request.ToString();
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const HttpResponse response) {
    os << response.ToString();
    return os;
}
}