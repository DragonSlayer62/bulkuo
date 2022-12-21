//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef exist_hpp
#define exist_hpp

#include <cstdint>
#include <string>
#include <functional>
#include <map>

#include "../argument.hpp"
//=================================================================================
extern std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> exist_mapping;


#endif /* exist_hpp */
