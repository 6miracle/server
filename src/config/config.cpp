#include "config.h"
namespace server {
std::unordered_map<std::string, ConfigVarBase::ptr> Config::ConfigMap;
template< typename T>
static std::string ToStr(const T& val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
}
static void ListAllMembers(const std::string& prefix, const YAML::Node node, std::vector<std::pair<std::string, const YAML::Node>>& vec) {
    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._1234567890") != std::string::npos) {
        LOG_ERROR("Config Invalid Name prefix = %s node = %s", prefix.c_str(), ToStr(node).c_str());
        return ;
    }
    vec.push_back({prefix, node});
    if(node.IsMap()) {
        // std::cout <<"------\n";
        for(auto i = node.begin(); i != node.end(); ++i) {
            ListAllMembers(prefix.empty() ? i->first.Scalar() : prefix + "." + i->first.Scalar(), i->second,vec);
        }
    }
}
// 加载Yaml文件
bool Config::LoadFromYaml(std::string path) {
    YAML::Node node = YAML::LoadFile(path);
    std::vector<std::pair<std::string, const YAML::Node>> vec;

    ListAllMembers("", node, vec);

    for(auto& i : vec) {
        std::string& key = i.first;
        // std::cout <<"key" << key <<'\n';
        if(key.empty()) continue;
        ConfigVarBase::ptr var = LookUp(key);
        if(var) {
            if(i.second.IsScalar()) {
                var->fromString(i.second.Scalar());
            } else {
                var->fromString(ToStr(i.second));
            }

        }
    }
    return true;
}

bool Config::TurnIntoYaml(std::string path) {
   std::ofstream os(path);
   YAML::Node node;
   for(auto i = ConfigMap.begin(); i != ConfigMap.end(); ++i) {
        node[i->first] = YAML::Load(i->second->toString());
   }
   os << node;
   os.close();
   return true;
}

}