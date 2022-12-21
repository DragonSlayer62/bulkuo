//Copyright © 2022 Charles Kerr. All rights reserved.

#include "hueaccess.hpp"

#include <iostream>
#include <vector>
using namespace std::string_literals;

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
namespace ultima {
    //=================================================================================
    auto hueOffset(std::uint32_t id) ->std::uint32_t {
        auto group = static_cast<uint32_t>(id)/8 ;
        auto offset = group*hue_group_size + 4 + ((static_cast<std::uint32_t>(id)%8)*hue_entry_size);
        return offset ;
    }
    //=================================================================================
    auto hueEntries(const std::filesystem::path &huefile) ->std::uint32_t{
        auto size = static_cast<std::uint32_t>(std::filesystem::file_size(huefile));
        auto groups = size/hue_group_size;
        return static_cast<std::uint32_t>(groups * 8) ;
    }
    //=================================================================================
    auto hueEntries(std::ifstream &input) ->std::uint32_t {
        auto current = input.tellg();
        input.seekg(0,std::ios::end);
        auto offset = input.tellg();
        input.seekg(current,std::ios::beg);
        auto groups = static_cast<std::uint32_t>(offset/hue_group_size);
        return groups*8 ;
        
    }
    //=================================================================================
    auto createHue(std::ofstream &output, std::uint16_t huecount ) ->std::uint32_t{
        output.seekp(0,std::ios::beg);
        auto buffer = std::vector<char>(hue_group_size,0) ;
        auto groups = static_cast<std::uint32_t>(huecount/8) ;
        
        for (std::uint32_t group = 0 ;group < groups ;group++){
            output.write(buffer.data(),buffer.size());
        }
        return static_cast<std::uint32_t>(groups * 8) ;
    }
    
    //=================================================================================
    auto hueData(std::ifstream &input, std::uint32_t id) ->std::vector<std::uint8_t> {
        auto buffer = std::vector<std::uint8_t>(ultima::hue_entry_size,0) ;
        auto offset = hueOffset(id) ;
        input.seekg(offset,std::ios::beg) ;
        if (!input.good()){
            throw std::runtime_error("Not in hue file, id: "s + std::to_string(id) );
        }
        input.read(reinterpret_cast<char*>(buffer.data()),buffer.size());
        if (input.gcount()!= buffer.size()){
            throw std::runtime_error("Not in hue file, id: "s + std::to_string(id) );
        }
        return buffer ;
    }
    
    //=================================================================================
    auto nameForHue(std::ifstream &input, std::uint32_t id) ->std::string {
        auto current = input.tellg();
        input.seekg(hueOffset(id)+68,std::ios::beg);
        auto buffer = std::vector<char>(21,0) ;
        input.read(buffer.data(),20);
        input.seekg(current,std::ios::beg);
        return buffer.data();
    }

}
