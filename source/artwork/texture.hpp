//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef texture_hpp
#define texture_hpp

#include <cstdint>
#include <string>
#include <vector>

#include "bitmap.hpp"

//=================================================================================
// Take texture data and return a bimap
auto bitmapForTexture(const std::vector<std::uint8_t> &data) ->bitmap_t<std::uint16_t> ;

//=================================================================================
// Take texture image and return data
auto dataForTexture(const bitmap_t<std::uint16_t> &image) ->std::vector<std::uint8_t> ;

#endif /* texture_hpp */
