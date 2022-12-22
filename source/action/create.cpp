//Copyright © 2022 Charles Kerr. All rights reserved.

#include "create.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "strutil.hpp"
#include "actutility.hpp"
#include "uophash.hpp"

#include "../uodata/uop.hpp"
#include "../uodata/hash.hpp"
#include "../uodata/idx.hpp"
#include "../uodata/uowave.hpp"
#include "../uodata/multicollection.hpp"


#include "../artwork/bitmap.hpp"
#include "../artwork/gump.hpp"
#include "../artwork/art.hpp"
#include "../artwork/texture.hpp"

using namespace std::string_literals;

//==================================================================================
// Forward decleares
//=================================================================================

auto createUOP(const argument_t &arg,datatype_t type) ->void;
auto createIDXMul(const argument_t &arg,datatype_t type) ->void;
auto createUOPIDXMul(const argument_t &arg,datatype_t type) ->void;


//=================================================================================
auto noCreate(const argument_t &args,datatype_t type) ->void {
    auto name = nameForDatatype(type);
    throw std::runtime_error("Action: 'create'is not supported for type: "s+name);
}
//==================================================================================
// Constants used
//=================================================================================

//=================================================================================
std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> create_mapping{
    {datatype_t::art,createUOPIDXMul},{datatype_t::info,noCreate},
    {datatype_t::texture,createIDXMul},{datatype_t::sound,createUOPIDXMul},
    {datatype_t::gump,createUOPIDXMul},{datatype_t::animation,noCreate},
    {datatype_t::hue,noCreate},{datatype_t::multi,createUOPIDXMul}
};

//================================================================================
auto createUOP(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()!=2){
        throw std::runtime_error("Invalid number of paths, format is: directory uopoutput");
    }
    auto directory = arg.paths.at(0);
    auto outputpath = arg.paths.at(1);
    auto housingpath = std::filesystem::path();
    auto housing = std::ifstream();
    auto data = contentsFor(directory, primaryForType(type) );
    auto validids = validInContents(arg, data);
    if (validids.empty()){
        std::runtime_error("No valid ids found in: "s + directory.string());
    }
    auto count = static_cast<std::uint32_t>(validids.size());
    auto fileflag = std::ios::binary ;
    if (type == datatype_t::multi){
        // We have to add housing.bin
        // Lets see if we can find it
        housingpath = directory/std::filesystem::path("housing.bin");
        housing = std::ifstream(housingpath.string(),std::ios::binary);
        if (!housing.is_open()){
            throw std::runtime_error("Unable to open: "s + housingpath.string());
        }
        fileflag = std::ios::in;
        count += 1;
    }
    arg.writeOK(outputpath);
    auto output = std::ofstream(outputpath.string(),std::ios::binary);
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s+outputpath.string());
    }
    try{
        auto offsets = ultima::createUOP(output, count);
        auto buffer = std::vector<std::uint8_t>();
        auto bitmap = bitmap_t<std::uint16_t>();
        auto [hashformat,maxid] = getUOPInfoFor(type);
        auto index = 0 ;
        for (const auto &id:validids){
            // This is the valid id
            auto path = data.at(id) ;
            auto input = std::ifstream(path.string(),fileflag);
            if (!input.is_open()){
                output.close();
                std::filesystem::remove(outputpath);
                throw std::runtime_error("Unable to open: "s + path.string());
            }
            auto entry = ultima::table_entry();
            auto hash = strutil::format(hashformat,id);
            entry.identifier = ultima::hashLittle2(hash);
            createUOPEntry(type,id, path,input,buffer, entry);
            entry.compressed_length =static_cast<std::uint32_t>(buffer.size());
            entry.data_block_hash=ultima::hashAdler32(buffer);
            entry.offset = static_cast<std::uint64_t>(output.tellp());
            output.write(reinterpret_cast<char*>(buffer.data()),data.size());
            auto current = output.tellp();
            output.seekp(offsets.at(index));
            index++;
            entry.save(output);
            output.seekp(current,std::ios::beg);
        }
        if (type == datatype_t::multi){
            // WE have to do housing.bin
            housing.seekg(0,std::ios::end);
            auto size = static_cast<std::uint64_t>(housing.tellg());
            housing.seekg(0,std::ios::beg);
            auto buffer = std::vector<std::uint8_t>(size,0);
            housing.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
            auto entry = ultima::table_entry();
            entry.identifier = ultima::hashLittle2("build/multicollection/housing.bin");
            entry.compression = 1 ;
            entry.decompressed_length = static_cast<std::uint32_t>(buffer.size());
            entry.offset = static_cast<std::uint64_t>(output.tellp());
            buffer = ultima::compressUOPData(buffer);
            entry.compressed_length=static_cast<std::uint32_t>(buffer.size());
            output.write(reinterpret_cast<char*>(buffer.data()),buffer.size());
            output.seekp(offsets.at(index));
            entry.save(output);
        }
    }
    catch(...){
        output.close();
        std::filesystem::remove(outputpath);
        throw;
    }
}
//================================================================================
auto createIDXMul(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()!=3){
        throw std::runtime_error("Invalid number of paths, format is: directory idxoutput muloutput");
    }
    auto directory = arg.paths.at(0);
    auto idxpath = arg.paths.at(1);
    auto mulpath = arg.paths.at(2);
    auto data = contentsFor(directory, primaryForType(type) );
    auto validids = validInContents(arg, data);
    if (validids.empty()){
        std::runtime_error("No valid ids found in: "s + directory.string());
    }
    auto count =  std::max(minIDXForType(type),*(validids.rbegin())) + 1;
    auto fileflag = std::ios::binary ;
    if (type == datatype_t::multi){
        fileflag = std::ios::in;
    }
    arg.writeOK(idxpath);
    arg.writeOK(mulpath);
    auto idx = std::ofstream(idxpath.string(),std::ios::binary);
    if (!idx.is_open()){
        throw std::runtime_error("Unable to create: "s+idxpath.string());
    }
    auto mul = std::ofstream(mulpath.string(),std::ios::binary);
    if (!mul.is_open()){
        idx.close();
        std::filesystem::remove(idxpath);
        throw std::runtime_error("Unable to create: "s+mulpath.string());
    }
    auto idxoffsets = ultima::createIDX(idx, count);
    auto bitmap = bitmap_t<std::uint16_t>();
    auto buffer = std::vector<std::uint8_t>();
    try{
        for (std::uint32_t id = 0 ; id < count;id++){
            auto entry = ultima::idx_t() ;
            entry.offset=0xFFFFFFFF;
            entry.length = 0 ;
            if (type==datatype_t::art){
                entry.extra=0xFFFFFFFF;
            }
            if (validids.find(id) != validids.end()){
                entry.offset = static_cast<std::uint32_t>(mul.tellp());
                entry.extra = 0 ;
                auto path = data.at(id);
                auto input = std::ifstream(path.string(),fileflag);
                if (!input.is_open()){
                    throw std::runtime_error("Unable to open: "s + path.string());
                }
                createIDXEntry(type, id,path, input,buffer,entry)   ;
                entry.length = static_cast<std::uint32_t>(buffer.size());
                mul.write(reinterpret_cast<char*>(buffer.data()),buffer.size());
            }
            idx.seekp(idxoffsets.at(id),std::ios::beg);
            entry.save(idx);
        }
    }
    catch(...){
        idx.close();
        mul.close();
        std::filesystem::remove(idxpath);
        std::filesystem::remove(mulpath);
        throw;
    }
    
}
//================================================================================
auto createUOPIDXMul(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()==3){
        createIDXMul(arg, type);
    }
    else if (arg.paths.size()==2){
        createUOP(arg,type);
    }
    else{
        throw (std::runtime_error("Invalid number of paths."));
    }
}
