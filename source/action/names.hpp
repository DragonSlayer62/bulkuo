//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef names_hpp
#define names_hpp

#include <cstdint>
#include <string>
#include <functional>
#include <map>

#include "../argument.hpp"
//=================================================================================
extern std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> names_mapping;

#endif /* names_hpp */
