//Copyright © 2022 Charles Kerr. All rights reserved.

#include "hash.hpp"

#include <iostream>
#include <vector>
#include <fstream>

using namespace std::string_literals;
namespace ultima{
    //=================================================================================
    auto hashLittle2(const std::string &hashstring) ->std::uint64_t {
        auto a = std::uint32_t(0) ;
        auto b = std::uint32_t(0) ;
        auto c = std::uint32_t(0) ;
        
        auto k = int(0) ;
        
        auto length = static_cast<std::uint32_t>(hashstring.size()) ;
        
        c = 0xDEADBEEF + static_cast<std::uint32_t>(length) ;
        a = c;
        b = c ;
        
        while (length > 12){
            a += (hashstring[k]);
            a += (hashstring[k+1] << 8);
            a += (hashstring[k+2] << 16);
            a += (hashstring[k+3] << 24);
            b += (hashstring[k+4]);
            b += (hashstring[k+5] << 8);
            b += (hashstring[k+6] << 16);
            b += (hashstring[k+7] << 24);
            c += (hashstring[k+8]);
            c += (hashstring[k+9] << 8);
            c += (hashstring[k+10] << 16);
            c += (hashstring[k+11] << 24);
            
            a -= c; a ^= c << 4 | c >> 28; c += b;
            b -= a; b ^= a << 6 | a >> 26; a += c;
            c -= b; c ^= b << 8 | b >> 24; b += a;
            a -= c; a ^= c << 16 | c >> 16; c += b;
            b -= a; b ^= a << 19 | a >> 13; a += c;
            c -= b; c ^= b << 4 | b >> 28; b += a;
            
            length -= 12 ;
            k += 12;
        }
        
        if (length != 0){
            // Notice the lack of breaks!  we actually want it to fall through
            switch (length) {
                case 12:
                    c += (hashstring[k+11] << 24);
                    [[fallthrough]];
                case 11:
                    c += (hashstring[k+10] << 16);
                    [[fallthrough]];
                case 10:
                    c += (hashstring[k+9] << 8);
                    [[fallthrough]];
                case 9:
                    c += (hashstring[k+8]);
                    [[fallthrough]];
                case 8:
                    b += (hashstring[k+7] << 24);
                    [[fallthrough]];
                case 7:
                    b += (hashstring[k+6] << 16);
                    [[fallthrough]];
                case 6:
                    b += (hashstring[k+5] << 8);
                    [[fallthrough]];
                case 5:
                    b += (hashstring[k+4]);
                    [[fallthrough]];
                case 4:
                    a += (hashstring[k+3] << 24);
                    [[fallthrough]];
                case 3:
                    a += (hashstring[k+2] << 16);
                    [[fallthrough]];
                case 2:
                    a += (hashstring[k+1] << 8);
                    [[fallthrough]];
                case 1: {
                    a += (hashstring[k]);
                    c ^= b;
                    c -= (b << 14) | (b >> 18);
                    a ^= c;
                    a -= (c << 11) | (c >> 21);
                    b ^= a;
                    b -= (a << 25) | (a >> 7);
                    c ^= b;
                    c -= (b << 16) | (b >> 16);
                    a ^= c;
                    a -= (c << 4)  | (c >> 28);
                    b ^= a;
                    b -= (a << 14) | (a >> 18);
                    c ^= b;
                    c -= (b << 24) | (b >> 8);
                    [[fallthrough]];
                }
                default:
                    break;
            }
        }
        return (static_cast<std::uint64_t>(b) << 32) | static_cast<std::uint64_t>(c) ;
        
    }
    
    //==================================================================================
    auto hashAdler32(const std::vector<std::uint8_t> &data) ->std::uint32_t {
        std::uint32_t a = 1 ;
        std::uint32_t b = 0 ;
        for (const auto &entry : data ) {
            a = (a + static_cast<std::uint32_t>(entry)) % 0xFFF1;
            b = (b + a) % 0xFFF1 ;
        }
        return (b<<16)| a ;
        
    }
    //==================================================================================
    auto hashAdler32(std::iostream &input,std::uint32_t amount) ->std::uint32_t {
        std::uint32_t a = 1 ;
        std::uint32_t b = 0 ;
        auto entry = std::uint8_t(0);
        for (std::uint32_t j = 0; j<amount ;j++) {
            input.read(reinterpret_cast<char*>(&entry),sizeof(entry));
            a = (a + static_cast<std::uint32_t>(entry)) % 0xFFF1;
            b = (b + a) % 0xFFF1 ;
        }
        return (b<<16)| a ;
        
    }
    //==================================================================================
    hashset_t::hashset_t(const std::string &format,std::uint32_t startnum,std::uint32_t endnum):hashset_t(){
        load(format,startnum,endnum);
    }
    //==================================================================================
    auto hashset_t::clear() ->void {
        hashes.clear() ;
    }
    //==================================================================================
    auto hashset_t::load(const std::string &format,std::uint32_t startnum,std::uint32_t endnum) ->void {
        for (std::uint32_t entry = startnum; entry <= endnum;entry+=1){
            auto hashformat = applyformat(format,entry) ;
            hashes.insert_or_assign(hashLittle2(hashformat),entry);
        }
    }
    //==================================================================================
    auto hashset_t::size() const ->size_t {
        return hashes.size();
    }
    //==================================================================================
    auto hashset_t::insert(std::uint64_t hash, std::uint32_t entry) ->void {
        hashes.insert_or_assign(hash,entry);
    }
    //==================================================================================
    auto hashset_t::operator[](std::uint64_t hash) const -> const std::uint32_t& {
        return hashes.at(hash) ;
    }
    //==================================================================================
    auto hashset_t::operator[](std::uint64_t hash)  ->  std::uint32_t& {
        return hashes.at(hash) ;
    }
}
