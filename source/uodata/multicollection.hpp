//

#ifndef multicollection_hpp
#define multicollection_hpp

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <zlib.h>
#include <fstream>
#include <limits>

#include "hash.hpp"
#include "idx.hpp"
#include "uop.hpp"
namespace ultima{
    
    //=================================================================================
    //
    // Read multicollection.uop or multi mul/idx files
    //
    //================================================================================
    
    //================================================================================
    // Item contined in a multi entry
    struct multi_sub_item_t {
        static const std::string header_line ;
        static constexpr auto min_size = size_t(14) ;
        static constexpr auto mul_record_size = size_t(16) ;
        std::uint16_t tileid ;
        std::int16_t xoffset ;
        std::int16_t yoffset ;
        std::int16_t zoffset ;
        std::uint64_t flag ;
        std::vector<std::uint32_t> cliloc;
        multi_sub_item_t();
        multi_sub_item_t(const std::string &line);
        auto load(const std::uint8_t *data,bool isuop) ->size_t ;
        auto data(bool isuop ) const ->std::vector<std::uint8_t> ;
        auto description(bool use_hex) const ->std::string ;
     };
    
    //================================================================================
    struct multi_entry_t {
        static const std::map<std::uint32_t,std::string> names ;
        std::vector<multi_sub_item_t> items ;
        std::int16_t minx ;
        std::int16_t miny ;
        std::int16_t minz ;
        std::int16_t maxx ;
        std::int16_t maxy ;
        std::int16_t maxz ;
        multi_entry_t() :minz(std::numeric_limits<std::int16_t>::max()),maxz(std::numeric_limits<std::int16_t>::min()),miny(std::numeric_limits<std::int16_t>::max()),maxy(std::numeric_limits<std::int16_t>::min()),minx(std::numeric_limits<std::int16_t>::max()),maxx(std::numeric_limits<std::int16_t>::min()){}
        multi_entry_t(const std::vector<std::uint8_t> &data, bool isuop);
        auto data(bool isuop) const ->std::vector<std::uint8_t> ;
        auto load(const std::vector<std::uint8_t> &data, bool isuop) ->void ;
        auto load(const std::filesystem::path &path) ->void ;
        auto load(std::ifstream &input)->void ;
        auto description(const std::filesystem::path &path,bool use_hex) ->void ;
        auto description(std::ofstream &output,bool use_hex) ->void;
    };
    

}
#endif /* multicollection_hpp */
