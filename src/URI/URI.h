#pragma once
#include "pch.h"
#include "Address/Address.h"
namespace server {

class Uri {
public:
    using ptr = std::shared_ptr<Uri>;
    /*
     foo://user@sylar.com:8042/over/there?name=ferret#nose
       \_/   \______________/\_________/ \_________/ \__/
        |           |            |            |        |
     scheme     authority       path        query   fragment
    */

    Uri();
    static Uri::ptr Create(const std::string&) ;

    Address::ptr CreateAddress() const;
    std::string ToString() const;

    const std::string& getScheme() const { return scheme_; }
    const std::string& getUserinfo() const { return userinfo_; }
    const std::string& getHost() const { return host_; }
    int32_t getPort() const;
    const std::string& getPath() const ;
    const std::string& getQuery() const { return query_; }
    const std::string& getFragment() const { return fragment_; }

    void setScheme(const std::string& scheme) { scheme_ = scheme; }
    void setUserinfo(const std::string& userinfo) { userinfo_ = userinfo; }
    void setHost(const std::string& host) { host_ = host; }
    void setPort(int32_t port) { port_ = port; }
    void setPath(const std::string& path)  { path_ = path; }
    void setQuery(const std::string& query) { query_ = query; }
    void setFragment(const std::string& fragment) { fragment_ = fragment; }

    bool isDefaultPort() const;
private:
    std::string scheme_;
    std::string userinfo_;
    std::string host_;
    int32_t port_;
    std::string path_;
    std::string query_;
    std::string fragment_;
};
}