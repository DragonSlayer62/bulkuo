//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef light_hpp
#define light_hpp

#include <cstdint>
#include <string>
#include <vector>

#include "bitmap.hpp"
//=================================================================================

auto bitmapForLight(std::uint32_t width , std::uint32_t light,const std::vector<std::uint8_t> &data ) ->bitmap_t<std::uint16_t> ;
auto dataForLight(const bitmap_t<std::uint16_t> &bitmap) ->std::vector<std::uint8_t> ;
#endif /* light_hpp */
