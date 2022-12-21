//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef merge_hpp
#define merge_hpp

#include <cstdint>
#include <string>
#include <functional>
#include <map>

#include "../argument.hpp"
//=================================================================================
extern std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> merge_mapping;


#endif /* merge_hpp */
