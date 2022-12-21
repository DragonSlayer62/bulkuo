//Copyright © 2022 Charles Kerr. All rights reserved.

#include "flag.hpp"

#include <iostream>

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
        {"background"s,0x1},{"weapon"s,0x2},{"transparent"s,0x4},{"translucent"s,0x8},
        {"wall"s,0x10},{"damaging"s,0x20},{"impassable"s,0x40},{"wet"s,0x80},
        {"bit9"s,0x100},{"surface"s,0x200},{"climbable"s,0x400},{"stackable"s,0x800},
        {"window"s,0x1000},{"noshoot"s,0x2000},{"articlea"s,0x4000},{"articlean"s,0x8000},
        {"articlethe"s,0x10000},{"foliage"s,0x20000},{"partialhue"s,0x40000},{"nohouse"s,0x80000},
        {"map"s,0x100000},{"container"s,0x200000},{"wearable"s,0x400000},{"lightsource"s,0x800000},
        {"animation"s,0x1000000},{"hoverover"s,0x2000000},{"nodiagnol"s,0x4000000},{"armor"s,0x8000000},
        {"roof"s,0x10000000},{"door"s,0x20000000},{"stairback"s,0x40000000},{"stairright"s,0x80000000},
        {"alphablend"s,0x100000000},{"usenewart"s,0x200000000},{"artused"s,0x400000000},{"bit36"s,0x800000000},
        {"noshadow"s,0x1000000000},{"pixelbleed"s,0x2000000000},{"playanimonce"s,0x4000000000},{"bit40"s,0x8000000000},
        {"multimovable"s,0x10000000000},{"bit42"s,0x20000000000},{"bit43"s,0x40000000000},{"bit44"s,0x80000000000},
        {"bit45"s,0x100000000000},{"bit46"s,0x200000000000},{"bit47"s,0x400000000000},{"bit48"s,0x800000000000},
        {"bit49"s,0x1000000000000},{"bit50"s,0x2000000000000},{"bit51"s,0x4000000000000},{"bit52"s,0x8000000000000},
        {"bit53"s,0x10000000000000},{"bit54"s,0x20000000000000},{"bit55"s,0x40000000000000},{"bit56"s,0x80000000000000},
        {"bit57"s,0x100000000000000},{"bit58"s,0x200000000000000},{"bit59"s,0x400000000000000},{"bit60"s,0x800000000000000},
        {"bit61"s,0x1000000000000000},{"bit62"s,0x2000000000000000},{"bit63"s,0x4000000000000000},{"bit64"s,0x8000000000000000}
    };
    
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
        auto working = flagvalue;
        auto current = std::string();
        while (!working.empty()){
            auto pos = working.find(sep) ;
            
            if (pos!= std::string::npos){
                current = working.substr(0,pos) ;
                if (pos+1 < working.size()){
                    working = working.substr(pos+1) ;
                }
                else {
                    working="";
                }
            }
            else {
                current = working ;
                working ="";
            }
            value |= flagname_masks.at(current) ;
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
        auto rvalue = std::string();
        auto one = std::uint64_t(1) ;
        for (auto j=0; j<64;j++) {
            auto mask = (one<<j) ;
            if ( (mask&value)!= 0) {
                if (!rvalue.empty()){
                    rvalue+=sep ;
                }
                rvalue += flagNameForBit(j+1) ;
            }
        }
        
        return rvalue ;
    }
    //*********************************************************************************
}
