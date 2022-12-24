//Copyright © 2022 Charles Kerr. All rights reserved.

#include "actutility.hpp"

#include <iostream>
#include <map>
#include <stdexcept>
#include <fstream>
#include "strutil.hpp"

#include "../artwork/bitmap.hpp"
#include "../artwork/gump.hpp"
#include "../artwork/art.hpp"
#include "../artwork/texture.hpp"
#include "../artwork/light.hpp"

#include "../uodata/multicollection.hpp"
#include "../uodata/uowave.hpp"

using namespace std::string_literals;

//=================================================================================
static const std::map<datatype_t,std::string> primary_extensions{
    {datatype_t::art,".bmp"s},{datatype_t::gump,".bmp"s},
    {datatype_t::sound,".wav"s},{datatype_t::hue,".bmp"s},
    {datatype_t::multi,".csv"s},{datatype_t::texture,".bmp"s},
    {datatype_t::light,".bmp"s}
};
//=================================================================================
static const std::map<datatype_t,std::string> secondary_extensions{
    {datatype_t::sound,".txt"s},{datatype_t::hue,".txt"s}
};

//=================================================================================
auto primaryForType(datatype_t type) ->std::string {
    auto iter = primary_extensions.find(type);
    if (iter != primary_extensions.end()){
        return iter->second;
    }
    return "";
}
//=================================================================================
auto secondaryForType(datatype_t type) ->std::string {
    auto iter = secondary_extensions.find(type);
    if (iter != secondary_extensions.end()){
        return iter->second;
    }
    return "";
}
//=================================================================================
auto idFromString(const std::string &value) ->std::uint32_t {
    auto id = std::uint32_t(0);
    if (!value.empty()){
        auto pos = value.find_first_not_of("0") ;
        if (pos!= std::string::npos){
            
            if ((value.at(pos) == 'x') || (value.at(pos)=='X')){
                // THis is a hex value ;
                id = static_cast<std::uint32_t>(std::stoul(value,nullptr,0)) ;
            }
            else {
                id =static_cast<std::uint32_t>(std::stoul(value.substr(pos),nullptr,0)) ;
            }
        }
    }
    return id ;
}
//====================================================================================
auto contentsFor(const std::filesystem::path &path,const std::string &extension) ->std::map<std::uint32_t,std::filesystem::path> {
    auto iter = std::filesystem::recursive_directory_iterator(path) ;
    auto rvalue = std::map<std::uint32_t,std::filesystem::path>() ;
    for (auto &entry:iter){
        if (std::filesystem::is_regular_file(entry.path())){
            if (strutil::lower(entry.path().extension().string()) == extension){
                auto [first,second] = strutil::split(entry.path().stem().string(),"-") ;
                try{
                    auto id = idFromString(first);
                    rvalue.insert_or_assign(id,entry.path());
                }
                catch(...){
                    // Ok, we counld't make a id from it, so we skip it.
                }
            }
        }
    }
    return rvalue ;
}
//====================================================================================
auto validInContents(const argument_t &arg, const std::map<std::uint32_t,std::filesystem::path> &data)->std::set<std::uint32_t>{
    auto rvalue = std::set<std::uint32_t>();
    for (const auto &[id,path]:data){
        if(arg.id(id)){
            rvalue.insert(id) ;
        }
    }
    return rvalue ;
}
//====================================================================================
auto nameInFile(const std::filesystem::path &path) ->std::string {
    // This reads on line from the input
    auto buffer = std::vector<char>(4096,0);
    auto input = std::ifstream(path.string());
    auto line = std::string();
    if (input.is_open()){
        input.getline(buffer.data(), 4095);
        if (input.gcount()>0){
            std::string line = buffer.data();
            strutil::trim(line);
        }
    }
    return line;
}
//====================================================================================
auto unionOfId(const std::set<std::uint32_t> &ids, const std::map<std::uint32_t,ultima::table_entry> &mapping) ->std::set<std::uint32_t> {
    auto rvalue = ids ;
    for (const auto &[id,entry]:mapping){
        if (rvalue.find(id)== rvalue.end()){
            rvalue.insert(id);
        }
    }
    return rvalue ;
}
//====================================================================================
static std::map<datatype_t,std::uint32_t> min_idx_tileid {
    {datatype_t::gump,0xFFFe},{datatype_t::art,0x13ffd},
    {datatype_t::sound,0xFFE},{datatype_t::multi,0x21ff},
    {datatype_t::texture,0x3FFF},{datatype_t::animation,0},
    {datatype_t::hue,2999},{datatype_t::light,99}
};
//====================================================================================
auto minIDXForType(datatype_t type) ->std::uint32_t {
    auto iter = min_idx_tileid.find(type);
    if (iter != min_idx_tileid.end()){
        return iter->second;
    }
    return 0;
}

//====================================================================================
auto createUOPEntry(datatype_t type, std::uint32_t id, std::filesystem::path &path,std::ifstream &input,std::vector<std::uint8_t> &buffer, ultima::table_entry &entry)->void{
    auto bitmap = bitmap_t<std::uint16_t>();
    switch(type) {
        case datatype_t::gump:{
            bitmap = bitmap_t<std::uint16_t>::fromBMP(input);
            buffer = dataForGump(bitmap);
            entry.decompressed_length = static_cast<std::uint32_t>(buffer.size());
            
            break;
        }
        case datatype_t::art:{
            bitmap = bitmap_t<std::uint16_t>::fromBMP(input);
            if (id <0x4000){
                buffer  = dataForTerrain(bitmap);
            }
            else {
                buffer = dataForItem(bitmap);
            }
            entry.decompressed_length = static_cast<std::uint32_t>(buffer.size());
            
            break;
        }
        case datatype_t::sound:{
            auto txtpath = path;
            txtpath.replace_extension(".txt") ;
            auto name = nameInFile(txtpath);
            
            auto wav = ultima::uowave_t(input);
            buffer = wav.createUO(name);
            entry.decompressed_length =static_cast<std::uint32_t>(buffer.size());
            break;
        }
        case datatype_t::multi:{
            auto multi = ultima::multi_entry_t();
            multi.load(input);
            buffer = multi.data(true);
            entry.decompressed_length = static_cast<std::uint32_t>(buffer.size());
            buffer = ultima::compressUOPData(buffer);
            entry.compression=1;
            break;
        }
        default:{
            throw std::runtime_error("Uop creation not supported.");
        }
    }
}
//====================================================================================
auto createIDXEntry(datatype_t type, std::uint32_t id,std::filesystem::path &path, std::ifstream &input,std::vector<std::uint8_t> &buffer, ultima::idx_t &entry) ->void{
    auto bitmap = bitmap_t<std::uint16_t>();
    switch(type) {
        case datatype_t::gump:{
            bitmap = bitmap_t<std::uint16_t>::fromBMP(input);
            auto [width,height] = bitmap.size();
            std::copy(reinterpret_cast<std::uint8_t*>(&height),reinterpret_cast<std::uint8_t*>(&height)+2,reinterpret_cast<std::uint8_t*>(&entry.length));
            std::copy(reinterpret_cast<std::uint8_t*>(&width),reinterpret_cast<std::uint8_t*>(&width)+2,reinterpret_cast<std::uint8_t*>(&entry.length)+2);
            buffer = dataForGump(bitmap);
            break;
        }
        case datatype_t::texture:{
            bitmap = bitmap_t<std::uint16_t>::fromBMP(input);
            auto [width,height] = bitmap.size();
            entry.extra = (width==128?1:0);
            buffer = dataForTexture(bitmap);
            
            break;
        }
        case datatype_t::art:{
            bitmap = bitmap_t<std::uint16_t>::fromBMP(input);
            if (id < 0x4000){
                buffer = dataForTerrain(bitmap);
            }
            else {
                buffer = dataForItem(bitmap);
            }
            break;
        }
        case datatype_t::light: {
            bitmap =bitmap_t<std::uint16_t>::fromBMP(input);
            auto [width,height] = bitmap.size();
            entry.extra = (width&0xFFFF) | ((width&0xFFFF)<<16);
            buffer = dataForLight(bitmap);
        }
        case datatype_t::sound:{
            entry.extra = id +1 ;
            auto wav = ultima::uowave_t(input);
            auto secondpath = path ;
            secondpath.replace_extension(".txt");
            auto name = nameInFile(secondpath);
            buffer = wav.createUO(name);
            break;
        }
        case datatype_t::multi:{
            auto multi = ultima::multi_entry_t() ;
            multi.load(input);
            buffer = multi.data(false);
            break;
        }
        default:{
            throw std::runtime_error("Type does not support idx/mul creation.");
        }
            
    }

}
//====================================================================================
auto updateInfo(const argument_t &arg, std::ifstream &input, ultima::tileinfo_t &info)->std::uint32_t{
    auto buffer = std::vector<char>(4098,0);
    auto count = 0 ;
    auto amount_processed = std::uint32_t(0);
    while(input.good() && !input.eof()){
        count++;
        input.getline(buffer.data(),4097);
        if (input.gcount()>0){
            buffer.at(input.gcount())=0;
            std::string line = buffer.data();
            line = strutil::trim(strutil::strip(line,"//"));
            if (!line.empty()){
                if ((line.at(0) >= 48) && (line.at(0)<= 57)){
                    // This is a good line, maybe
                    auto [sid,rest] = strutil::split(line,",");
                    try {
                        auto id = static_cast<std::uint32_t>(std::stoul(sid,nullptr,0));
                        if (arg.id(id)){
                            amount_processed++;
                            if (id <0x4000){
                                auto tile = ultima::landtile_t(rest, ",");
                                info.land(id) = tile;
                            }
                            else{
                                auto tile = ultima::itemtile_t(rest, ",");
                                info.item(id-0x4000) = tile;
                            }
                        }
                    }
                    catch(...){
                        // We do nothing, if we can convert it, we just move one.
                        std::cerr <<"Error processing , skipping line: "<<count<<std::endl;
                    }
                    
                }
            }
        }
    }
    return amount_processed;
}
