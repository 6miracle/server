#include "Servlet.h"
#include <fnmatch.h>
#include <memory>
#include <mutex>

namespace server {

FuncServlet::FuncServlet(callback cb): cb_(cb) {}
int32_t FuncServlet::Handle(HttpRequest::ptr& request, HttpResponse::ptr& response, HttpSession::ptr& session) {
    return cb_(request, response, session);
}

int32_t NotFoundServlet::Handle(HttpRequest::ptr& , HttpResponse::ptr&, HttpSession::ptr&)  {
    return 0;
}
int32_t ServletDispatch::Handle(HttpRequest::ptr& request, HttpResponse::ptr& response, HttpSession::ptr& session) {
    Servlet::ptr slt = GetMatchServlet(request->GetPath());
    if(slt) {
        LOG_INFO("servelt%s uri = %s", slt->ToString().c_str(), request->GetPath().c_str());
        slt->Handle(request, response, session);
    }
    return 0;
}
void ServletDispatch::addServlet(const std::string& uri, Servlet::ptr slt) {
    std::lock_guard<std::mutex> lock(mtx_);
    data_.insert({uri, slt});
}
void ServletDispatch::addServlet(const std::string& uri, FuncServlet::callback call) {
     std::lock_guard<std::mutex> lock(mtx_);
    data_.insert({uri, std::make_shared<FuncServlet>(call)});
}
void ServletDispatch::addGlobServlet(const std::string& uri, Servlet::ptr slt) {
     std::lock_guard<std::mutex> lock(mtx_);
    for(auto& m : global_) {
        if(m.first == uri) { 
            m.second = slt;
            return ;
        }
    }
    global_.push_back({uri, slt});

}
void ServletDispatch::addGlobServlet(const std::string& uri, FuncServlet::callback call) {
     std::lock_guard<std::mutex> lock(mtx_);
    for(auto& m : global_) {
        if(m.first == uri) {
            m.second = std::make_shared<FuncServlet>(call);
            return ;
        }
    }
    global_.push_back({uri, std::make_shared<FuncServlet>(call)});
}

void ServletDispatch::delServlet(std::string& uri) {
     std::lock_guard<std::mutex> lock(mtx_);
    data_.erase(uri);
}
void ServletDispatch::delGlobServlet(std::string& uri) {
    std::lock_guard<std::mutex> lock(mtx_);
    for(auto p = global_.begin(); p != global_.end(); ++p) {
        if(p->first == uri) {
            global_.erase(p);
            return ;
        }
    }
}

Servlet::ptr ServletDispatch::GetServelt(const std::string& uri) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto tmp = data_.find(uri);
    if(tmp != data_.end()) {
        Servlet::ptr servlet = tmp->second;
        return servlet;
    }
    return nullptr;
    // return nullptr;
    // Servlet::ptr servlet = data_.find(uri)->second;
    // return servlet ? servlet : nullptr;
}
Servlet::ptr ServletDispatch::GetGlobServlet(const std::string& uri) {
    std::lock_guard<std::mutex> lock(mtx_);
    for(auto p : global_) {
        if(p.first == uri) {
            return p.second;
        }
    }
    return nullptr;
}
Servlet::ptr ServletDispatch::GetMatchServlet(const std::string& uri) {
    Servlet::ptr ret = GetServelt(uri);
    std::lock_guard<std::mutex> lock(mtx_);
    if(!ret) { 
        for(auto p : global_) {
            if(!fnmatch(p.first.c_str(), uri.c_str(), 0)) {
                ret = p.second;
                LOG_INFO("matched sevelt %s urr = %s", p.first.c_str(), uri.c_str());
                break;
            }
        }
    }

    return ret;
}
}