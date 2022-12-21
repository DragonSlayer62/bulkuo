//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef hueaccess_hpp
#define hueaccess_hpp

#include <cstdint>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector> 
namespace ultima {
    constexpr std::uint32_t hue_entry_size = 88 ;
    constexpr std::uint32_t hue_group_size = (8 * hue_entry_size ) + 4 ;
    //=================================================================================
    auto hueOffset(std::uint32_t id) ->std::uint32_t ;
    auto hueEntries(const std::filesystem::path &huefile) ->std::uint32_t;
    auto hueEntries(std::ifstream &input) ->std::uint32_t ;
    auto createHue(std::ofstream &output, std::uint16_t huecount = 3000) ->std::uint32_t;
    auto hueData(std::ifstream &input, std::uint32_t id) ->std::vector<std::uint8_t> ;
    auto nameForHue(std::ifstream &input, std::uint32_t id) ->std::string ;
}
#endif /* hueaccess_hpp */
