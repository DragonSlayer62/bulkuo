//Copyright © 2022 Charles Kerr. All rights reserved.

#include "uophash.hpp"

#include <iostream>
#include <stdexcept>


using namespace std::string_literals;

//=================================================================================
static const std::map<datatype_t,std::pair<std::string,std::uint32_t>> hash_information {
    {datatype_t::art,{"build/artlegacymul/%08u.tga"s,0x4000+0xFFFF}},{datatype_t::sound,{"build/soundlegacymul/%08u.dat"s,0x10000}},
    {datatype_t::gump,{"build/gumpartlegacymul/%08u.tga"s,0x10000}},{datatype_t::multi,{"build/multicollection/%06u.bin"s,0x10000}}
};

//=================================================================================
auto getUOPInfoFor(datatype_t type) -> std::pair<std::string,std::uint32_t> {
    auto iter = hash_information.find(type);
    if (iter != hash_information.end()){
        return iter->second;
    }
    throw std::runtime_error("No uop hash information for '"s + nameForDatatype(type)+"'."s);
}
