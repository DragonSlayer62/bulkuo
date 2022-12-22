//Copyright © 2022 Charles Kerr. All rights reserved.

#include "flag.hpp"

#include <iostream>
#include <algorithm>
#include "strutil.hpp"
using namespace std::string_literals;

//=================================================================================
//=================================================================================
namespace ultima {
    //=================================================================================
    // Define flag_t
    //=================================================================================
    //=================================================================================
    // Define flagnames
    //=================================================================================
    const std::map<std::string,std::uint64_t> flag_t::flagname_masks{
        {"background"s,0x1ull},{"weapon"s,0x2ull},{"transparent"s,0x4ull},{"translucent"s,0x8ull},
        {"wall"s,0x10ull},{"damaging"s,0x20ull},{"impassable"s,0x40ull},{"wet"s,0x80ull},
        {"bit9"s,0x100ull},{"surface"s,0x200ull},{"climbable"s,0x400ull},{"stackable"s,0x800ull},
        {"window"s,0x1000ull},{"noshoot"s,0x2000ull},{"articlea"s,0x4000ull},{"articlean"s,0x8000ull},
        {"articlethe"s,0x10000ull},{"foliage"s,0x20000ull},{"partialhue"s,0x40000ull},{"nohouse"s,0x80000ull},
        {"map"s,0x100000ull},{"container"s,0x200000ull},{"wearable"s,0x400000ull},{"lightsource"s,0x800000ull},
        {"animation"s,0x1000000ull},{"hoverover"s,0x2000000ull},{"nodiagnol"s,0x4000000ull},{"armor"s,0x8000000ull},
        {"roof"s,0x10000000ull},{"door"s,0x20000000ull},{"stairback"s,0x40000000ull},{"stairright"s,0x80000000ull},
        {"alphablend"s,0x100000000ull},{"usenewart"s,0x200000000ull},{"artused"s,0x400000000ull},{"bit36"s,0x800000000ull},
        {"noshadow"s,0x1000000000ull},{"pixelbleed"s,0x2000000000ull},{"playanimonce"s,0x4000000000ull},{"bit40"s,0x8000000000ull},
        {"multimovable"s,0x10000000000ull},{"bit42"s,0x20000000000ull},{"bit43"s,0x40000000000ull},{"bit44"s,0x80000000000ull},
        {"bit45"s,0x100000000000ull},{"bit46"s,0x200000000000ull},{"bit47"s,0x400000000000ull},{"bit48"s,0x800000000000ull},
        {"bit49"s,0x1000000000000ull},{"bit50"s,0x2000000000000ull},{"bit51"s,0x4000000000000ull},{"bit52"s,0x8000000000000ull},
        {"bit53"s,0x10000000000000ull},{"bit54"s,0x20000000000000ull},{"bit55"s,0x40000000000000ull},{"bit56"s,0x80000000000000ull},
        {"bit57"s,0x100000000000000ull},{"bit58"s,0x200000000000000ull},{"bit59"s,0x400000000000000ull},{"bit60"s,0x800000000000000ull},
        {"bit61"s,0x1000000000000000ull},{"bit62"s,0x2000000000000000ull},{"bit63"s,0x4000000000000000ull},{"bit64"s,0x8000000000000000ull}
    };
    //================================================================================
    auto flag_t::flag_header(const std::string &sep) ->std::string {
        auto label = std::string();
        for (auto j=0 ; j<64;j++){
            auto mask = std::uint64_t(1)<<j ;
            auto iter=std::find_if(flagname_masks.begin(),flagname_masks.end(),[mask](const std::pair<std::string,std::uint64_t> &value){
                return std::get<1>(value) == mask ;
            });
            if (iter != flagname_masks.end()){
                if (!label.empty()){
                    label += sep ;
                }
                label += iter->first;
            }
        }
        return label ;
    }
    
    //=================================================================================
    // Define flag_t::maskForName
    //=================================================================================
    //=================================================================================
    auto flag_t::maskForName(const std::string &flag_name) ->std::uint64_t {
        auto iter = flagname_masks.find(flag_name) ;
        if (iter != flagname_masks.end()){
            return iter->second ;
        }
        throw std::out_of_range("Flag name: "s+flag_name+" not valid"s);
    }
    
    //=================================================================================
    // Define flag_t::flagNameForBit
    //=================================================================================
    //=================================================================================
    auto flag_t::flagNameForBit(int bit) ->const std::string& {
        auto mask = std::uint64_t(1) ;
        mask = mask<<(bit-1) ;
        auto iter = std::find_if(flagname_masks.begin(),flagname_masks.end(),[mask](const std::pair<std::string ,std::uint64_t> &value){
            return mask == std::get<1>(value);
        });
        if (iter != flagname_masks.end()){
            return iter->first;
        }
        throw std::out_of_range("Bit value not between 1 and 64 inclusive: "s + std::to_string(bit));
    }
    
    //=================================================================================
    // Initalize with a string
    //=================================================================================
    //=================================================================================
    flag_t::flag_t(const std::string &flagvalue, const std::string &sep):value(0){
        auto values = strutil::parse(flagvalue,sep);
        if (values.size() != 64){
            throw std::runtime_error("Invalid bits for flag initialization");
        }
        for (auto j=0 ; j<64;j++){
            auto mask = std::uint64_t(1)<<j;
            if ((values.at(j)=="0") || values.at(j).empty()){
                value &= (~mask) ;
            }
            else {
                value |= mask;
            }
        }
    }
    
    
    //=================================================================================
    // Define bit clear/set methods (for non mask use, bits 0-63)
    //=================================================================================
    //=================================================================================
    auto flag_t::setBit(int bit,bool value) ->void {
        auto mask = (std::uint64_t(1) << bit) ;
        if (value) {
            this->value |= mask;
        }
        else {
            this->value &= (~mask) ;
        }
    }
    //=================================================================================
    auto flag_t::bitValue(int bit) const ->bool {
        auto mask = (std::uint64_t(1) << bit) ;
        return value&mask ;
    }
    
    //=================================================================================
    // Some convience methods so you dont have to do a ".value"
    //=================================================================================
    //=================================================================================
    auto flag_t::operator=(const std::uint64_t &value) ->flag_t& {
        this->value = value ;
        return *this ;
    }
    //=================================================================================
    auto flag_t::operator==(const std::uint64_t &value) const ->bool {
        return this->value == value ;
    }
    
    //=================================================================================
    // Text representation of the value
    //=================================================================================
    //=================================================================================
    auto flag_t::description(const std::string &sep) const ->std::string {
        
        auto label = std::string();
        for (auto j=0 ; j<64;j++){
            auto mask = std::uint64_t(1)<<j ;
            if (!label.empty()){
                label += sep;
            }
            if (value & mask){
                label += "1";
            }
            else{
                label += " ";
            }
       }
        return label ;
    }
    //*********************************************************************************
}
