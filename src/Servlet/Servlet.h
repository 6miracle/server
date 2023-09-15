#pragma once
#include "Http/Http.h"
#include "Http/HttpSession.h"
#include <memory>
#include <mutex>
#include <unordered_map>

namespace server {
class Servlet {
public:
    using ptr = std::shared_ptr<Servlet>;
    virtual ~Servlet() = default;

    // 处理请求
    virtual int32_t Handle(HttpRequest::ptr& , HttpResponse::ptr&, HttpSession::ptr&) = 0;

    virtual const std::string ToString() const { return "Servlet"; }
private:

};

class NotFoundServlet: public Servlet {
public:
    using ptr = std::shared_ptr<NotFoundServlet>;
    using callback = std::function<int32_t (HttpRequest::ptr, HttpResponse::ptr, HttpSession::ptr)>;

    int32_t Handle(HttpRequest::ptr& , HttpResponse::ptr&, HttpSession::ptr&) override;
    const std::string ToString() const override { return "NotFoundServlet"; }
private:

};


class FuncServlet: public Servlet {
public:
    using ptr = std::shared_ptr<FuncServlet>;
    using callback = std::function<int32_t (HttpRequest::ptr, HttpResponse::ptr, HttpSession::ptr)>;

    FuncServlet(callback cb);

    int32_t Handle(HttpRequest::ptr& , HttpResponse::ptr&, HttpSession::ptr&) override;
    const std::string ToString() const override { return "FuncServlet"; }
private:    
    callback cb_;
};

class ServletDispatch: public Servlet {
public:
    using ptr = std::shared_ptr<ServletDispatch>;

    void addServlet(const std::string& uri, Servlet::ptr slt);
    void addServlet(const std::string& uri, FuncServlet::callback call);
    void addGlobServlet(const std::string& uri, Servlet::ptr slt);
    void addGlobServlet(const std::string& uri, FuncServlet::callback call);

    void delServlet(std::string& uri);
    void delGlobServlet(std::string& uri);

    void setDefault(Servlet::ptr v) { default_ = v; }
    Servlet::ptr getDefault() { return default_; }

    Servlet::ptr GetServelt(const std::string& uri);
    Servlet::ptr GetGlobServlet(const std::string& uri);
    Servlet::ptr GetMatchServlet(const std::string& uri);

    int32_t Handle(HttpRequest::ptr& , HttpResponse::ptr&, HttpSession::ptr&) override;
    const std::string ToString() const override { return "ServletDispatcher"; }
private:    
    std::unordered_map<std::string, Servlet::ptr> data_;
    std::vector<std::pair<std::string, Servlet::ptr>> global_; // 模糊查询
    Servlet::ptr default_;
    std::mutex mtx_;
 
};
}