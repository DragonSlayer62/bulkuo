//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef animation_hpp
#define animation_hpp

#include <cstdint>
#include <string>
#include <vector>
#include <tuple>

#include "bitmap.hpp"

//=================================================================================
auto decipherAnimData(const std::vector<std::uint8_t> &data) ->std::vector<std::tuple<std::int16_t,std::int16_t,bitmap_t<std::uint16_t>>> ;
auto decipherAnimFrame(const std::uint8_t *data,std::uint32_t length,const std::vector<std::uint16_t> &palette) ->std::tuple<std::int16_t , std::int16_t , bitmap_t<std::uint16_t>> ;


#endif /* animation_hpp */
