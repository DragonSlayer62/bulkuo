//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef uophash_hpp
#define uophash_hpp

#include <cstdint>
#include <string>
#include <map>
#include <utility>

#include "../types.hpp"
//=================================================================================
auto getUOPInfoFor(datatype_t type) -> std::pair<std::string,std::uint32_t> ;

#endif /* uophash_hpp */
