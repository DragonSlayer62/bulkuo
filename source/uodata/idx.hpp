//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef idx_hpp
#define idx_hpp

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <fstream>
namespace ultima {
    //=================================================================================
    struct idx_t {
        std::uint32_t offset ;
        std::uint32_t length ;
        std::uint32_t extra ;
        idx_t(std::uint32_t offset_value = 0xFFFFFFFF) :offset(offset_value),length(0),extra(0){}
        idx_t(std::ifstream &input,std::uint32_t offset_value = 0xFFFFFFFF) : idx_t(offset_value){ load(input);}
        auto lowExtra() const ->std::uint16_t { return static_cast<std::uint16_t>(extra&0x0000FFFF);}
        auto lowExtra(std::uint16_t value) ->void { extra |= static_cast<std::uint32_t>(value)  ;}
        auto highExtra() const ->std::uint16_t { return static_cast<std::uint16_t>((extra>>16)&0x0000FFFF);}
        auto highExtra(std::uint16_t value) ->void { extra |= (static_cast<std::uint32_t>(value) << 16) ;}
        auto valid() const ->bool { return (offset<0xFFFFFFFE) && (length>0) ;}
        auto load(std::ifstream &input) ->void;
        auto save(std::ofstream &output) ->void;
    };
    auto gatherIDXOffsets(std::ifstream &input) ->std::vector<size_t> ;
    auto gatherIDXEntries( std::ifstream &input) ->std::map<std::uint32_t,idx_t> ;
    auto createIDX(std::ofstream &output, std::uint32_t numentries,std::uint32_t def_invalid_offset = 0xFFFFFFFF)->std::vector<std::uint64_t> ;
    
    auto readMulData(std::ifstream &input,idx_t entry) ->std::vector<std::uint8_t> ;
}
#endif /* idx_hpp */
