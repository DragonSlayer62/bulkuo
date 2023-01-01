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
#include "../uodata/hueaccess.hpp"
#include "../uodata/tileinfo.hpp"

#include "../artwork/bitmap.hpp"
#include "../artwork/hue.hpp"
#include "../artwork/animation.hpp"

using namespace std::string_literals;

//==================================================================================
// Forward decleares
//=================================================================================

auto createUOP(const argument_t &arg,datatype_t type) ->void;
auto createIDXMul(const argument_t &arg,datatype_t type) ->void;
auto createUOPIDXMul(const argument_t &arg,datatype_t type) ->void;
auto createHue(const argument_t &arg,datatype_t type) ->void;
auto createInfo(const argument_t &arg,datatype_t type) ->void ;
auto createAnimationMul(const argument_t &arg,datatype_t type) ->void ;


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
    {datatype_t::art,createUOPIDXMul},{datatype_t::info,createInfo},
    {datatype_t::texture,createIDXMul},{datatype_t::sound,createUOPIDXMul},
    {datatype_t::gump,createUOPIDXMul},{datatype_t::animation,createAnimationMul},
    {datatype_t::hue,createHue},{datatype_t::multi,createUOPIDXMul}
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
    auto amount_processed = std::uint32_t(0);
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
        output.seekp(0,std::ios::end); // Go the end of the file!
        auto buffer = std::vector<std::uint8_t>();
        auto bitmap = bitmap_t<std::uint16_t>();
        auto [hashformat,maxid] = getUOPInfoFor(type);
        auto index = 0 ;
        for (const auto &id:validids){
            amount_processed++;
            // This is the valid id
            auto path = data.at(id) ;
            auto input = std::ifstream(path.string(),fileflag);
            if (!input.is_open()){
                output.close();
                std::filesystem::remove(outputpath);
                throw std::runtime_error("Unable to open: "s + path.string());
            }
            auto entry = ultima::table_entry();
            entry.compression = 0 ;
            auto hash = strutil::format(hashformat,id);
            entry.identifier = ultima::hashLittle2(hash);
            auto compress = createUOPEntry(type,id, path,input,buffer);
            
            
            entry.decompressed_length =static_cast<std::uint32_t>(buffer.size());
            if (compress){
                buffer = ultima::compressUOPData(buffer);
                entry.compression = 1 ;
            }
            entry.compressed_length =static_cast<std::uint32_t>(buffer.size());
            entry.data_block_hash=ultima::hashAdler32(buffer);
            entry.offset = static_cast<std::uint64_t>(output.tellp());
            output.write(reinterpret_cast<char*>(buffer.data()),buffer.size());
            
            auto current = output.tellp();
            output.seekp(offsets.at(index),std::ios::beg);
            index++;
            entry.save(output);
            output.seekp(current,std::ios::beg);
        }
        if (type == datatype_t::multi){
            amount_processed++;
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
        std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;
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
    auto amount_processed = std::uint32_t(0);
    try{
        for (std::uint32_t id = 0 ; id < count;id++){
            auto entry = ultima::idx_t() ;
            entry.offset=0xFFFFFFFF;
            entry.length = 0 ;
            if ((type==datatype_t::art)||(type == datatype_t::animation)){
                entry.extra=0xFFFFFFFF;
            }
            if (validids.find(id) != validids.end()){
                amount_processed++;
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
        std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;

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
//================================================================================
auto createHue(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()!= 2){
        throw std::runtime_error("Invalid number of paths, format: directory outhuemul");
    }
    auto directory = arg.paths.at(0);
    auto huepath = arg.paths.at(1);
    arg.writeOK(huepath);
    auto potential = contentsFor(directory, primaryForType(type));
    auto validid = validInContents(arg, potential);
    auto count = std::max(*validid.rbegin(),minIDXForType(type)) +1;
    auto output = std::ofstream(huepath.string(),std::ios::binary);
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s +huepath.string());
    }
    ultima::createHue(output,count);
    auto amount_processed = std::uint32_t(0);
    for (const auto &id : validid){
        auto path = potential.at(id) ;
        auto input = std::ifstream(path.string(),std::ios::binary);
        if (!input.is_open()){
            throw std::runtime_error("Unable to open: "s+path.string());
        }
        amount_processed++;
        auto namepath = path ;
        namepath.replace_extension(".txt") ;
        auto name = nameInFile(namepath);
        auto bitmap = bitmap_t<std::uint16_t>::fromBMP(input);
        auto buffer = dataFromHue(bitmap, name);
        auto offset = ultima::hueOffset(id);
        output.seekp(offset,std::ios::beg) ;
        output.write(reinterpret_cast<char*>(buffer.data()),buffer.size());
    }
    std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;

}
//================================================================================
auto createInfo(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()!= 2){
        throw std::runtime_error("Invalid number of paths, format: csvfile outtilemul");
    }
    auto inputpath = arg.paths.at(0);
    auto outpath = arg.paths.at(1);
    arg.writeOK(outpath);
    auto input = std::ifstream(inputpath.string());
    if (!input.is_open()){
        throw std::runtime_error("Unable to open: "s+inputpath.string());
    }
    auto info = ultima::tileinfo_t() ;
    auto amount_processed = updateInfo(arg, input,  info) ;
    info.save(outpath);
    std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;
}
//================================================================================
auto createAnimationMul(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()<3){
        throw std::runtime_error("Invalid number of paths, format is: csv_bmp_directory [replaceable_directory] idxoutput muloutput");
    }
    auto directory = arg.paths.at(0) ;
    auto second_directory = std::filesystem::path() ;
    auto pathindex = 1 ;
    if (arg.paths.size()==4){
        second_directory=arg.paths.at(pathindex);
        pathindex++;
    }
    auto idxpath = arg.paths.at(pathindex) ;
    pathindex++;
    auto mulpath = arg.paths.at(pathindex) ;
    auto anims = contentsFor(directory, ".csv") ;
    auto replaceable = std::map<std::uint32_t,std::filesystem::path>() ;
    if (!second_directory.empty()){
        replaceable = contentsFor(second_directory, ".swapped");
    }
    if (anims.empty()) {
        throw std::runtime_error("Found no animations (.csv) in : "s+directory.string()) ;
    }
    auto valid = validInContents(arg, anims) ;
    if (valid.empty()){
        throw std::runtime_error("No ids in selected list found in : "s+directory.string()) ;
    }
    auto numentries = *valid.rbegin() + 1 ;
    arg.writeOK(idxpath);
    arg.writeOK(mulpath);
    auto amount_processed = 0 ;

    auto idx = std::ofstream(idxpath.string(),std::ios::binary) ;
    if (!idx.is_open()){
        throw std::runtime_error("Unable to create: "s +idxpath.string());
    }
    auto mul = std::ofstream(mulpath.string(),std::ios::binary);
    if (!mul.is_open()){
        idx.close();
        std::filesystem::remove(idxpath);
        throw std::runtime_error("Unable to create: "s +mulpath.string());
    }
    for (std::uint32_t id = 0 ; id < numentries;id++){
        auto entry = ultima::idx_t() ;
        if (arg.id(id)){
            auto iter = anims.find(id);
            if (iter != anims.end()){
                auto input = std::ifstream(iter->second.string());
                if (!input.is_open()){
                    idx.close();
                    mul.close();
                    std::filesystem::remove(idxpath);
                    std::filesystem::remove(mulpath);
                    throw std::runtime_error("Unable to open: "s + iter->second.string());
                }
                auto anim = animation_t(input, iter->second);
                auto buffer = anim.data() ;
                entry.offset = static_cast<std::uint32_t>(mul.tellp()) ;
                entry.length =static_cast<std::uint32_t>(buffer.size());
                entry.extra = 0 ;
                mul.write(reinterpret_cast<char*>(buffer.data()),buffer.size());
                amount_processed++;
            }
            else {
                auto iter = replaceable.find(id) ;
                if (iter != replaceable.end()){
                    // It is!
                    // for now, so no
                    entry.offset = 0x0 ;
                    entry.extra = 0x0 ;
                    entry.length = 0x0 ;
                }
                else {
                    // for now, so no
                    entry.offset = 0xFFFFFFFF ;
                    entry.extra = 0xFFFFFFFF ;
                    entry.length = 0xFFFFFFFF ;
                    
                }

            }
        }
        else {
            // This is not a valid id to be included, so it is swappable?
            auto iter = replaceable.find(id) ;
            if (iter != replaceable.end()){
                // It is!
                // for now, so no
                entry.offset = 0x0 ;
                entry.extra = 0x0 ;
                entry.length = 0x0 ;
           }
            else {
                // for now, so no
                entry.offset = 0xFFFFFFFF ;
                entry.extra = 0xFFFFFFFF ;
                entry.length = 0xFFFFFFFF ;

            }
        }
        entry.save(idx);
        
    }
    std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;

}

