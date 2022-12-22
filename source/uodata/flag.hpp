//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef flag_hpp
#define flag_hpp

#include <cstdint>
#include <string>
#include <map>
//=================================================================================
//=================================================================================
namespace ultima {
    
    //=================================================================================
    // Define a flag structure for the 64 bit flag value.  Mainly to provide some
    // helper methods, and flag names.
    //=================================================================================
    struct flag_t {
        static auto flag_header(const std::string &sep) ->std::string ;
        static const std::map<std::string,std::uint64_t> flagname_masks ;
        static auto maskForName(const std::string &flag_name) ->std::uint64_t ;
        static auto flagNameForBit(int bit) ->const std::string& ;
        std::uint64_t value ;
        
        flag_t(std::uint64_t initial_value = 0) :value(initial_value){}
        flag_t(const std::string &flagvalue,const std::string &sep);
        // If you have a bit number (0-63), instead of a mask (which you can directly and/or to the value)
        // Use these methods
        auto setBit(int bit,bool value) ->void ; // bit is between 0 - 63
        auto bitValue(int bit) const ->bool ; // bit is between 0 - 63
        
        auto description(const std::string &sep ) const ->std::string ;
        
        auto operator=(const std::uint64_t &value) ->flag_t& ;
        auto operator==(const std::uint64_t &value) const ->bool ;
    };
}

#endif /* flag_hpp */
