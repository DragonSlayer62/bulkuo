//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef animsupport_hpp
#define animsupport_hpp

#include <cstdint>
#include <string>
#include <vector>

#include "../argument.hpp"

//=================================================================================
auto extractAnimation(const argument_t &arg, std::uint32_t id,const std::vector<std::uint8_t> &buffer )->void;

#endif /* animsupport_hpp */
