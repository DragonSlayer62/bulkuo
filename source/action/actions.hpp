//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef actions_hpp
#define actions_hpp

#include <cstdint>
#include <string>
#include <map>
#include <functional>

#include "../argument.hpp"

//=================================================================================

extern std::map<action_t,std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> &> action_mapping ;
#endif /* actions_hpp */
