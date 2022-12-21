//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef gump_hpp
#define gump_hpp

#include <cstdint>
#include <string>
#include <vector>

#include "bitmap.hpp"
//=================================================================================
//=================================================================================
auto bitmapForGump(const std::vector<std::uint8_t> &data) ->bitmap_t<std::uint16_t>;

//=================================================================================
auto dataForGump(const bitmap_t<std::uint16_t> &image) ->std::vector<std::uint8_t> ;


#endif /* gump_hpp */
