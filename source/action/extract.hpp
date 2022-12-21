//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef extract_hpp
#define extract_hpp

#include <cstdint>
#include <string>
#include <functional>
#include "../argument.hpp"

//=================================================================================
extern std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> extract_mapping;


#endif /* extract_hpp */
