//Copyright © 2022 Charles Kerr. All rights reserved.

#include "actutility.hpp"

#include <iostream>
#include <map>
#include <stdexcept>

#include "strutil.hpp"
using namespace std::string_literals;

//=================================================================================
static const std::map<datatype_t,std::string> primary_extensions{
    {datatype_t::art,".bmp"s},{datatype_t::gump,".bmp"s},
    {datatype_t::sound,".wav"s},{datatype_t::hue,".bmp"s},
    {datatype_t::multi,".csv"s}
};
//=================================================================================
static const std::map<datatype_t,std::string> secondary_extensions{
    {datatype_t::sound,".txt"s},{datatype_t::hue,".txt"s}
};

//=================================================================================
auto primaryForType(datatype_t type) ->std::string {
    auto iter = primary_extensions.find(type);
    if (iter != primary_extensions.end()){
        return iter->second;
    }
    return "";
}
//=================================================================================
auto secondaryForType(datatype_t type) ->std::string {
    auto iter = secondary_extensions.find(type);
    if (iter != secondary_extensions.end()){
        return iter->second;
    }
    return "";
}
//=================================================================================
auto idFromString(const std::string &value) ->std::uint32_t {
    auto id = std::uint32_t(0);
    if (!value.empty()){
        auto pos = value.find_first_not_of("0") ;
        if (pos!= std::string::npos){
            
            if ((value.at(pos) == 'x') || (value.at(pos)=='X')){
                // THis is a hex value ;
                id = static_cast<std::uint32_t>(std::stoul(value,nullptr,0)) ;
            }
            else {
                id =static_cast<std::uint32_t>(std::stoul(value.substr(pos),nullptr,0)) ;
            }
        }
    }
    return id ;
}
//====================================================================================
auto contentsFor(const std::filesystem::path &path,const std::string &extension) ->std::map<std::uint32_t,std::filesystem::path> {
    auto iter = std::filesystem::recursive_directory_iterator(path) ;
    auto rvalue = std::map<std::uint32_t,std::filesystem::path>() ;
    for (auto &entry:iter){
        if (std::filesystem::is_regular_file(entry.path())){
            if (strutil::lower(entry.path().extension().string()) == extension){
                auto [first,second] = strutil::split(entry.path().stem().string(),"-") ;
                try{
                    auto id = idFromString(first);
                    rvalue.insert_or_assign(id,entry.path());
                }
                catch(...){
                    // Ok, we counld't make a id from it, so we skip it.
                }
            }
        }
    }
    return rvalue ;
}
//====================================================================================
auto validInContents(const argument_t &arg, const std::map<std::uint32_t,std::filesystem::path> &data)->std::set<std::uint32_t>{
    auto rvalue = std::set<std::uint32_t>();
    for (const auto &[id,path]:data){
        if(arg.id(id)){
            rvalue.insert(id) ;
        }
    }
    return rvalue ;
}
//====================================================================================
auto nameInFile(const std::filesystem::path &path) ->std::string {
    // This reads on line from the input
    auto buffer = std::vector<char>(4096,0);
    auto input = std::ifstream(path.string());
    auto line = std::string();
    if (input.is_open()){
        input.getline(buffer.data(), 4095);
        if (input.gcount()>0){
            std::string line = buffer.data();
            strutil::trim(line);
        }
    }
    return line;
}
//====================================================================================
auto unionOfId(const std::set<std::uint32_t> &ids, const std::map<std::uint32_t,ultima::table_entry> &mapping) ->std::set<std::uint32_t> {
    auto rvalue = ids ;
    for (const auto &[id,entry]:mapping){
        if (rvalue.find(id)== rvalue.end()){
            rvalue.insert(id);
        }
    }
    return rvalue ;
}
//====================================================================================
static std::map<datatype_t,std::uint32_t> min_idx_tileid {
    {datatype_t::gump,0xFFFe},{datatype_t::art,0x13ffd},
    {datatype_t::sound,0xFFE},{datatype_t::multi,0x21ff},
    {datatype_t::texture,0x3FFF},{datatype_t::animation,0}
};
auto minIDXForType(datatype_t type) ->std::uint32_t {
    auto iter = min_idx_tileid.find(type);
    if (iter != min_idx_tileid.end()){
        return iter->second;
    }
    return 0;
}
