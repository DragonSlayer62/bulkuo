//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef hash_hpp
#define hash_hpp

#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
namespace ultima {
    //=================================================================================
    auto hashLittle2(const std::string &hashstring) ->std::uint64_t;
    auto hashAdler32(const std::vector<std::uint8_t> &data) ->std::uint32_t;
    auto hashAdler32(std::iostream &input,std::uint32_t amount) ->std::uint32_t;
    
    //==================================================================================
    // Until we migrate to c++20 with <format>, we have our own, that can format strings
    template<typename ... Args>
    std::string applyformat( const std::string& format_str, Args ... args )
    {
    auto rvalue = std::string() ;
    if (!format_str.empty()) {
        // First see how much space we need?
        auto size_s = std::snprintf( nullptr, 0, format_str.c_str(), args ... );
        if (size_s <0) {
            throw std::runtime_error("Error applying format string");
        }
        if (size_s > 0){
            // Take the space we need and add 1 for the terminating \0
            size_s += 1 ;
            auto size = static_cast<size_t>( size_s );
            // Lets create a buffer we need for the data
            auto buf = std::make_unique<char[]>( size );
            size_s = std::snprintf( buf.get(), size, format_str.c_str(), args ... );
            if (size_s <0) {
                throw std::runtime_error("Error applying format string");
            }
            if (size_s >0){
                rvalue = std::string( buf.get(), buf.get() + size_s);
            }
        }
    }
    return rvalue  ;
    
    }
    
    //========================================================================================
    class hashset_t {
        std::map<std::uint64_t,std::uint32_t> hashes ;
    public:
        hashset_t(const std::string &format,std::uint32_t startnum,std::uint32_t endnum);
        hashset_t() = default;
        auto clear() ->void ;
        auto load(const std::string &format,std::uint32_t startnum,std::uint32_t endnum) ->void ;
        auto size() const ->size_t ;
        auto insert(std::uint64_t hash, std::uint32_t entry) ->void ;
        auto operator[](std::uint64_t hash) const -> const std::uint32_t& ;
        auto operator[](std::uint64_t hash)  ->  std::uint32_t& ;
        
    };
}

#endif /* hash_hpp */
