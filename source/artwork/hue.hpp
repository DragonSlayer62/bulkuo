//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef hue_hpp
#define hue_hpp

#include <cstdint>
#include <string>
#include <vector>
#include <utility>

#include "bitmap.hpp"
//=================================================================================

auto dataFromHue(const bitmap_t<std::uint16_t> &bitmap, const std::string &optional) ->std::vector<std::uint8_t> ;

auto hueFromData(const std::vector<std::uint8_t> &data, int width = 5,int height = 5) ->std::pair<bitmap_t<std::uint16_t>,std::string> ;

auto hueBlank(const std::vector<std::uint8_t> &data) ->bool ;
#endif /* hue_hpp */
