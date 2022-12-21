//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef create_hpp
#define create_hpp

#include <cstdint>
#include <string>
#include <functional>
#include <map>

#include "../argument.hpp"
//=================================================================================
extern std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> create_mapping;


#endif /* create_hpp */
