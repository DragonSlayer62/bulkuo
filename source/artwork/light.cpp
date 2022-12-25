//Copyright © 2022 Charles Kerr. All rights reserved.

#include "light.hpp"

#include <iostream>

using namespace std::string_literals;

//=================================================================================
auto bitmapForLight(std::uint32_t width , std::uint32_t height,const std::vector<std::uint8_t> &data ) ->bitmap_t<std::uint16_t> {
    auto image = bitmap_t<std::uint16_t>(width,height) ;
    auto value = data.cbegin();
    for (std::uint32_t y=0 ; y<height;y++){
        for (std::uint32_t x=0 ; x<width ; x++){
            auto chan = static_cast<std::uint16_t>(*value) ;
            auto channel = chan & 0x1f;
            image.pixel(static_cast<int>(x),static_cast<int>(y)) = ((channel<<10) | (channel<<5) | (channel));
            value++;
        }
    }
    return image ;
}
//=================================================================================
auto dataForLight(const bitmap_t<std::uint16_t> &bitmap) ->std::vector<std::uint8_t> {
    auto [width,height]  = bitmap.size();
    auto data = std::vector<std::uint8_t>(width*height,0);
    auto value = data.begin();
    for (std::uint32_t y=0 ; y<static_cast<std::uint32_t>(height);y++){
        for (std::uint32_t x=0 ; x< static_cast<std::uint32_t>(width);x++){
            auto chan = static_cast<std::uint16_t>(*value) ;
            auto channel = chan & 0x1f;
            *value = channel ;
            value++;
        }
    }
    return data ;
    
}

