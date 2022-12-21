//Copyright © 2022 Charles Kerr. All rights reserved.

#include "hue.hpp"

#include <iostream>
#include <algorithm>
using namespace std::string_literals;

//=================================================================================
//=================================================================================
/*
 3.7 HUES.MUL
 Just read in HueGroups until you hit the end of the file. Note that large chunks of this file consist of garbage--OSI admits to this (something about a bug in their old code).
 
 If you want to look at the hues, check out this.
 
 Hues are applied to an image in one of two ways. Either all gray pixels are mapped to the specified color range (resulting in a part of an image changed to that color) or all pixels are first mapped to grayscale and then the specified color range is mapped over the entire image.
 
 HueEntry
 WORD ColorTable[32];
 WORD TableStart;
 WORD TableEnd;
 CHAR Name[20];
 
 HueGroup
 DWORD Header;
 HueEntry Entries[8];
 */
//=================================================================================
auto dataFromHue(const bitmap_t<std::uint16_t> &bitmap, const std::string &optional) ->std::vector<std::uint8_t> {
    auto [width,height] = bitmap.size() ;
    auto buffer = std::vector<std::uint8_t>(88,0) ;
    if (!optional.empty()){
        auto count = std::min(static_cast<size_t>(20),optional.size());
        std::copy(optional.c_str(),optional.c_str()+count,reinterpret_cast<char*>(buffer.data()+68));
    }
    auto size = (width/32);
    for (auto j=0;j<32;j++){
        auto color = bitmap.pixel(j*size + size/2, height/2) &0x7FFF;
        std::copy(reinterpret_cast<std::uint8_t*>(&color),reinterpret_cast<std::uint8_t*>(&color)+2,buffer.data()+(j*2));
    }
    std::copy(buffer.data(),buffer.data()+2,buffer.data()+64) ;
    std::copy(buffer.data()+62,buffer.data()+64,buffer.data()+66) ;
    
   return buffer ;
}

//=================================================================================
auto hueFromData(const std::vector<std::uint8_t> &data, int width,int height ) ->std::pair<bitmap_t<std::uint16_t>,std::string> {
    auto bitmap = bitmap_t<std::uint16_t>(width*32,height) ;
    auto color = std::uint16_t(0) ;
    for (auto j=0 ;j<32;j++){
        std::copy(data.data()+(j*2),data.data()+(j*2)+2,reinterpret_cast<std::uint8_t*>(&color));
       
        auto offset = width * j;
        for (auto x = 0 ; x<width;x++){
            for (auto y=0; y<height;y++){
                bitmap.pixel(x+offset,y) = color;
            }
        }
    }
    auto name = std::string();
    auto buf = std::vector<char>(2,0);
    for (auto j=0 ; j<20;j++){
        buf[0] = static_cast<char>(data.at(j+68));
        if (buf[0] < 31){
            break;
        }
        else {
            std::string add = buf.data();
            name += add ;
        }
    }
    return std::make_pair(bitmap, name);
}
//=================================================================================
auto hueBlank(const std::vector<std::uint8_t> &data) ->bool {
    auto rvalue =true ;
    auto color = std::uint16_t(0);
    for (auto j=0 ; j<32;j++){
        std::copy(data.data()+(j*2),data.data()+(j*2)+2, reinterpret_cast<std::uint8_t*>(&color));
        if ((color &0x7FFF) != 1){
            rvalue = false;
            break;
        }
    }
    return rvalue ;
}
