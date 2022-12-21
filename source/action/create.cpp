//Copyright © 2022 Charles Kerr. All rights reserved.

#include "create.hpp"

#include <iostream>

using namespace std::string_literals;

//=================================================================================
auto noCreate(const argument_t &args,datatype_t type) ->void {
    auto name = nameForDatatype(type);
    throw std::runtime_error("Action: 'create'is not supported for type: "s+name);
}
//==================================================================================
// Constants used
//=================================================================================

//=================================================================================
std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> create_mapping{
    {datatype_t::art,noCreate},{datatype_t::info,noCreate},
    {datatype_t::texture,noCreate},{datatype_t::sound,noCreate},
    {datatype_t::gump,noCreate},{datatype_t::animation,noCreate},
    {datatype_t::hue,noCreate},{datatype_t::multi,noCreate}
};

