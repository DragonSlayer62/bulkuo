//

#ifndef tileinfo_hpp
#define tileinfo_hpp

#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <map>

#include "flag.hpp"
//=================================================================================
namespace ultima {
    
    
    //=================================================================================
    // Define a flag structure for the 64 bit flag value.  Mainly to provide some
    // helper methods, and flag names.
    //=================================================================================
    struct tilebase_t {
        flag_t flag ;
        std::string name ;
        tilebase_t()=default ;
        static auto header() ->std::string ;
        virtual auto description(bool use_hex) ->std::string {return std::string();}
    };
    //=================================================================================
    // Define a land tile structure
    //=================================================================================
    struct landtile_t : public tilebase_t{
        std::uint16_t textureid ;
        landtile_t():tilebase_t(),textureid(0){}
        landtile_t(std::ifstream &input,bool isHS = true) ;
        landtile_t(const std::string &line,const std::string &sep);
        auto save(std::ofstream &output,bool isHS)->void;
        auto description(bool use_hex) ->std::string final;
    };
    
    //=================================================================================
    // Define a item tile structure
    //=================================================================================
    struct itemtile_t : public tilebase_t{
        std::uint8_t weight;
        std::uint8_t quality ;
        std::uint16_t misc_data ;
        std::uint8_t unknown2 ;
        std::uint8_t quantity ;
        std::uint16_t animid ;
        std::uint8_t unknown3;
        std::uint8_t hue ;
        std::uint16_t stacking_offset ;
        std::uint8_t height ;
        auto description(bool use_hex) ->std::string;
        itemtile_t():tilebase_t(),weight(0),quality(0),misc_data(0),unknown2(0),quantity(0),animid(0),unknown3(0),hue(0),stacking_offset(0),height(0){}
        itemtile_t(std::ifstream &input,bool isHS = true) ;
        itemtile_t(const std::string &line,const std::string &sep);
        auto save(std::ofstream &output,bool isHS) ->void;
    };

    //=================================================================================
    // tileinfo_t class to access tiledata.mul
    //=================================================================================
    
    //=================================================================================
    class tileinfo_t {
        std::vector<landtile_t> landtiles ;
        std::vector<itemtile_t> itemtiles ;
        constexpr static auto hs_size = size_t(3188736);
        
        auto loadLand(std::ifstream &input,bool is_hs) ->void ;
        auto loadItem(std::ifstream &input,bool is_hs) ->void ;
        
        auto saveLand(std::ofstream &output,bool is_hs) ->void ;
        auto saveItem(std::ofstream &output,bool is_hs) ->void ;
    public:
        tileinfo_t(const std::filesystem::path &filepath = std::filesystem::path()) ;
        auto load(const std::filesystem::path &filepath) ->bool ;
        auto create() ->void ;
        auto land(std::uint32_t tileid) const ->const landtile_t&;
        auto land(std::uint32_t tileid)  -> landtile_t&;
        auto item(std::uint32_t tileid) const ->const itemtile_t& ;
        auto item(std::uint32_t tileid) ->itemtile_t & ;
        auto save(const std::filesystem::path &path,bool is_hs=true) ->void ;
    };

}

#endif /* tileinfo_hpp */
