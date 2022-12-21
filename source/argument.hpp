//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef argument_hpp
#define argument_hpp

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <set>
#include <filesystem>

#include "types.hpp"
#include "catlabel.hpp"
class argument_t {
    std::set<std::uint32_t> valid_ids ;
    std::map<std::uint32_t,std::string> label_mapping ;
    std::vector<std::pair<std::string,std::string>> flag_and_value ;
    
    category_t categories ;
    label_t labels ;
    
    auto processFlags() ->void ;
    auto processID(const std::string &line) ->void;
public:
    argument_t(int argc, const char* argv[]);
    
    std::vector<std::filesystem::path> paths ;
    bool use_hex ;
    bool overwrite ;
    int colorsize ;
    bool version ;
    bool help ;
    action_t action ;
    std::string action_value ;
    datatype_t type ;
    auto id(std::uint32_t id) const ->bool ;
    std::pair<int,int> huesize ;
    
    auto label(std::uint32_t id) const ->const std::string& ;
    auto locationFor(std::uint32_t id, const std::filesystem::path &directory) const ->std::filesystem::path ;
    auto filepath(std::uint32_t id, const std::filesystem::path &directory, const std::string &extension) const ->std::filesystem::path ;
    auto writeOK(const std::filesystem::path &path,bool exception=true) const ->bool;
};

#endif /* argument_hpp */
