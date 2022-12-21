//Copyright © 2022 Charles Kerr. All rights reserved.

#include "actions.hpp"

#include <iostream>

using namespace std::string_literals;

#include "extract.hpp"
#include "merge.hpp"
#include "create.hpp"
#include "names.hpp"
#include "exist.hpp"
//=================================================================================

std::map<action_t,std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> &> action_mapping{
    {action_t::extract,extract_mapping},{action_t::create,create_mapping},
    {action_t::merge,merge_mapping},{action_t::name,names_mapping},
    {action_t::exist,exist_mapping}
};

//=================================================================================

