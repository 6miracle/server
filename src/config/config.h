#ifndef CONFIG_SERVER_H
#define CONFIG_SERVER_H

#include <memory>
#include <string>
#include <yaml-cpp/yaml.h>
#include "pch.h"

namespace server{

// 配置项基类
class ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVarBase>;  

    ConfigVarBase(const std::string& name, const std::string& description): name_(name), description_(description) {}
    virtual ~ConfigVarBase() = default;

    const std::string getName() const { return name_; }
    const std::string getDescription() const { return description_; }

    virtual std::string toString()  = 0;
    virtual bool  fromString(const std::string& str) = 0;

    virtual std::string getTypeName() const = 0;

private:
    std::string name_;
    std::string description_;
};

#pragma region "转换"
// 转换基类
template<typename F, typename T>
class LexicalCast {
public:
    void operator()(const F& v) {
        LOG_ERROR("INVALID TYPE");
    }
};

template <>
class LexicalCast<std::string, double> {
public:
    double operator()(const std::string& str) {
        return ::atof(str.c_str());
    }
};
template <>
class LexicalCast<double, std::string> {
public:
    std::string operator()(const double v) {
        return std::to_string(v);
    }
};
template <>
class LexicalCast<std::string, int> {
public:
    double operator()(const std::string& str) {
        return ::atoi(str.c_str());
    }
};
template <>
class LexicalCast<int, std::string> {
public:
    std::string operator()(const int v) {
        return std::to_string(v);
    }
};


template<typename T>
class LexicalCast<T, std::string> {
public:
    std::string operator()(const T& v) {
        return std::to_string(v);
    }
};

template<typename T>
class LexicalCast<std::string, T> {
public:
    T operator() (const std::string& str) {
        return (T)std::stof(str);
    }
};
// std::vector
template<typename T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator()(const std::string& str) {
        std::vector<T> vec;
        YAML::Node node = YAML::Load(str);
        std::stringstream ss;
        node.SetStyle(YAML::EmitterStyle::Block);
        for(YAML::const_iterator i = node.begin(); i != node.end(); ++i) {
            ss.str("");
            ss << *i;
            vec.push_back(LexicalCast<std::string, T>(ss.str()));
        }
        return vec;
    }
};
template<typename T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& vec) {
        YAML::Node n(YAML::NodeType::Sequence);
        for(size_t i = 0; i < vec.size(); ++i) {
            n.push_back(YAML::Load(LexicalCast<T, std::string>(vec[i])));
        }
        std::stringstream ss;
        ss << n;
        return ss.str();
    }
};

// std::unordered_map
template<typename T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
    std::unordered_map<std::string, T> operator()(const std::string& str) {
        std::unordered_map<std::string, T> map;
        YAML::Node node = YAML::Load(str);
        std::stringstream ss;
        node.SetStyle(YAML::EmitterStyle::Block);
        for(YAML::const_iterator i = node.begin(); i != node.end(); ++i) {
            ss.str("");
            ss << i->second;
            map.insert({i->first.Scalar(), LexicalCast<std::string, T>(ss.str())});
        }
        return map;
    }
};
template<typename T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& map) {
        YAML::Node n(YAML::NodeType::Map);
        for(const auto i = map.begin(); i != map.end(); ++i) {
            n[i.first] = YAML::Load(LexicalCast<T, std::string>(i.second));
        }
        std::stringstream ss;
        ss << n;
        return ss.str();
    }
};
#pragma endregion


// 具体单个配置项
// 自定义类型
template <typename T,typename ToStr = LexicalCast<T, std::string>,
            typename FromStr = LexicalCast<std::string, T>>
class ConfigVar: public ConfigVarBase {
public:
    using  ptr = std::shared_ptr<ConfigVar>;

    ConfigVar(const std::string& name, const T& val, const std::string& description = ""): 
        ConfigVarBase(name, description), val_(val) {}

    ~ConfigVar() = default;

    // 转换为yaml字符串
    std::string toString() {
        YAML::Node node = YAML::Load(ToStr()(val_));
        std::stringstream ss;
        node.SetStyle(YAML::EmitterStyle::Block);
        // ss << getName() << ":" <<'\n';
        ss << node;
        return ss.str();
    }
    // 从yaml字符串进行解码
    bool fromString(const std::string& str) {
       setVal(FromStr()(str));
       return true;
    }

    // 获取类型名字
    std::string getTypeName() const {
        return typeid(T).name();
    }
    const T getVal() { return val_;}
    void setVal(const T& val) { val_ = val;}
private:
    T val_;
};

class Config {
public:
    bool LoadFromYaml(std::string path);
    bool TurnIntoYaml(std::string path);
    static ConfigVarBase::ptr LookUp(const std::string& name) {
        auto it = ConfigMap.find(name);
        if(it != ConfigMap.end()) {
            return it->second;
        }
        return nullptr;
    }
    template<typename T> static
    typename ConfigVar<T>::ptr LookUp(const std::string& name, const T& val, const std::string description = "") {
        auto it = ConfigMap.find(name);
        if(it != ConfigMap.end()) {
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
        } else {
            ConfigMap[name].reset(new ConfigVar<T>(name, val, description));
            return std::dynamic_pointer_cast<ConfigVar<T>>(ConfigMap[name]);
        }
    }
private:
    static std::unordered_map<std::string, ConfigVarBase::ptr> ConfigMap;
};
}


#endif