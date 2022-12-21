//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef art_hpp
#define art_hpp

#include <cstdint>
#include <string>
#include <vector>

#include "bitmap.hpp"
//=================================================================================
//=================================================================================
auto bitmapForTerrain(const std::vector<std::uint8_t> &data) ->bitmap_t<std::uint16_t> ;

//=================================================================================
auto dataForTerrain(const bitmap_t<std::uint16_t> &image) ->std::vector<std::uint8_t> ;
//=================================================================================
auto bitmapForItem(const std::vector<std::uint8_t> &data) ->bitmap_t<std::uint16_t> ;

//=================================================================================
auto dataForItem(const bitmap_t<std::uint16_t> &image) ->std::vector<std::uint8_t> ;

#endif /* art_hpp */
