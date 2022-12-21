//Copyright © 2022 Charles Kerr. All rights reserved.

#include "types.hpp"

#include <iostream>
#include <map>
#include <algorithm>
#include <utility>

using namespace std::string_literals;

//=================================================================================
// constants
//=================================================================================
//================================================================================
static const auto empty_string = std::string();

//=================================================================================
//  action_t
//=================================================================================
//=================================================================================
static const std::map<action_t,std::string> action_name_mapping{
    {action_t::unknown,"unknown"s},{action_t::name,"name"s},
    {action_t::exist,"exist"s},{action_t::create,"create"s},
    {action_t::merge,"merge"s},{action_t::extract,"extract"s}
};

//=================================================================================
auto nameForAction(action_t action) -> const std::string& {
    auto iter = action_name_mapping.find(action) ;
    if (iter != action_name_mapping.end()){
        return iter->second;
    }
    return empty_string;
}
//=================================================================================
auto actionForName(const std::string &name) ->action_t {
    auto iter = std::find_if(action_name_mapping.begin(),action_name_mapping.end(),[&name](const std::pair<action_t,std::string> &value){
        return name == std::get<1>(value) ;
    });
    if (iter != action_name_mapping.end()){
        return iter->first;
    }
    return action_t::unknown;
}


//=================================================================================
//  datatype_t
//=================================================================================
//=================================================================================
static const std::map<datatype_t,std::string> datatype_name_mapping{
    {datatype_t::unknown,"unknown"s},{datatype_t::art,"art"s},
    {datatype_t::texture,"texture"s},{datatype_t::sound,"sound"s},
    {datatype_t::gump,"gump"s},{datatype_t::hue,"hue"s},
    {datatype_t::multi,"multi"s},{datatype_t::info,"info"s},
    {datatype_t::animation,"animation"s}
};
//=================================================================================
auto nameForDatatype(datatype_t type) -> const std::string& {
    auto iter = datatype_name_mapping.find(type) ;
    if (iter != datatype_name_mapping.end()){
        return iter->second;
    }
    return empty_string;
}
//=================================================================================
auto datatypeForName(const std::string &name) ->datatype_t {
    auto iter = std::find_if(datatype_name_mapping.begin(),datatype_name_mapping.end(),[&name](const std::pair<datatype_t,std::string> &value){
        return name == std::get<1>(value) ;
    });
    if (iter != datatype_name_mapping.end()){
        return iter->first;
    }
    return datatype_t::unknown;
}

