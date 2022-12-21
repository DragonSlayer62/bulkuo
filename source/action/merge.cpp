//Copyright © 2022 Charles Kerr. All rights reserved.

#include "merge.hpp"

#include <iostream>

using namespace std::string_literals;

//=================================================================================
auto noMerge(const argument_t &args,datatype_t type) ->void {
    auto name = nameForDatatype(type);
    throw std::runtime_error("Action: 'merge'is not supported for type: "s+name);
}
//==================================================================================
// Constants used
//=================================================================================

//=================================================================================
std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> merge_mapping{
    {datatype_t::art,noMerge},{datatype_t::info,noMerge},
    {datatype_t::texture,noMerge},{datatype_t::sound,noMerge},
    {datatype_t::gump,noMerge},{datatype_t::animation,noMerge},
    {datatype_t::hue,noMerge},{datatype_t::multi,noMerge}
};
