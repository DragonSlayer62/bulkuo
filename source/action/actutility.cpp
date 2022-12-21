//Copyright © 2022 Charles Kerr. All rights reserved.

#include "actutility.hpp"

#include <iostream>
#include <map>
#include <stdexcept>
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
