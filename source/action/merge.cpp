//Copyright © 2022 Charles Kerr. All rights reserved.

#include "merge.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>

#include "uophash.hpp"
#include "actutility.hpp"

#include "../uodata/hash.hpp"
#include "../uodata/uop.hpp"
#include "../uodata/idx.hpp"
#include "../uodata/hueaccess.hpp"
#include "../uodata/tileinfo.hpp"

#include "../artwork/bitmap.hpp"
#include "../artwork/hue.hpp"
#include "../artwork/animation.hpp"

using namespace std::string_literals;


//===============================================================================
auto uopMerge(const argument_t &args,datatype_t type) ->void ;
auto idxmulMerge(const argument_t &args,datatype_t type) ->void ;
auto uopidxmulMerge(const argument_t &args,datatype_t type) ->void ;
auto hueMerge(const argument_t &args,datatype_t type) ->void ;
auto infoMerge(const argument_t &args,datatype_t type) ->void ;
auto mergeAnimationMul(const argument_t &arg,datatype_t type) ->void;
//=================================================================================
auto noMerge(const argument_t &args,datatype_t type) ->void {
    auto name = nameForDatatype(type);
    throw std::runtime_error("Action: 'merge'is not supported for type: "s+name);
}
//==================================================================================
// Constants used
//=================================================================================

//=================================================================================
std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> merge_mapping{
    {datatype_t::art,uopidxmulMerge},{datatype_t::info,infoMerge},
    {datatype_t::texture,idxmulMerge},{datatype_t::sound,uopidxmulMerge},
    {datatype_t::gump,uopidxmulMerge},{datatype_t::animation,mergeAnimationMul},
    {datatype_t::hue,hueMerge},{datatype_t::multi,uopidxmulMerge}
};
//===============================================================================
auto uopMerge(const argument_t &args,datatype_t type) ->void {
    if (args.paths.size()!= 2){
        throw std::runtime_error("Invalid number of paths, format: directory uoppath") ;
    }
    auto amount_processed = std::uint32_t(0);
    auto directory = args.paths.at(0);
    auto uoppath = args.paths.at(1);
    auto outputpath = uoppath;
    outputpath.replace_extension((outputpath.extension().string()+".bulkuo"s));
    args.writeOK(outputpath);
    auto data = contentsFor(directory, primaryForType(type));
    auto validids = validInContents(args, data);
    if (validids.empty()){
        throw std::runtime_error("No valid ids to merge.");
    }
    auto uop = std::ifstream(uoppath.string(),std::ios::binary) ;
    auto inoffsets = ultima::gatherEntryOffsets(uop);
    auto [hashformat,maxhashid] = getUOPInfoFor(type);
    auto hashet = ultima::hashset_t(hashformat, 0, maxhashid);
    auto inmapping = ultima::createIDTableMapping(uop, hashet, inoffsets);
    // Ok, so now, we can create our output
    auto realids = unionOfId(validids, inmapping);
    auto numberofids  = static_cast<std::uint32_t>(realids.size());
    auto fileflag = std::ios::binary ;
    if (type == datatype_t::multi){
        // we need to add housing
        numberofids++;
        fileflag = std::ios::in;
    }
    auto output = std::ofstream(outputpath.string(),std::ios::binary);
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s + outputpath.string()) ;
    }
    try{
        auto offsets = ultima::createUOP(output, numberofids);
        auto index = 0 ;
        auto buffer = std::vector<std::uint8_t>();
        for (const auto &id : realids){
            amount_processed++;
            auto entry = ultima::table_entry();
            entry.offset = static_cast<std::uint64_t>(output.tellp());
            if (validids.find(id) != validids.end()){
                // This was in the new ones
                auto path = data.at(id) ;
                auto input = std::ifstream(path.string(),fileflag);
                if (!input.is_open()){
                    output.close();
                    std::filesystem::remove(outputpath);
                    throw std::runtime_error("Unable to open: "s + path.string());
                }
                auto compress = createUOPEntry(type,id, path,input,buffer);
               
                
                entry.decompressed_length =static_cast<std::uint32_t>(buffer.size());
                if (compress){
                    buffer = ultima::compressUOPData(buffer);
                    entry.compression = 1 ;
                }

            }
            else {
                uop.seekg(inmapping.at(id).offset + inmapping.at(id).header_length,std::ios::beg);
                buffer.resize(inmapping.at(id).compressed_length,0);
                uop.read(reinterpret_cast<char*>(buffer.data()),buffer.size());
                entry.decompressed_length = inmapping.at(id).compressed_length;
                entry.compression = inmapping.at(id).compression;
                
            }
            
            entry.compressed_length = static_cast<std::uint32_t>(buffer.size());
            entry.data_block_hash=ultima::hashAdler32(buffer);
            output.write(reinterpret_cast<char*>(buffer.data()),buffer.size());
            auto current = output.tellp();
            output.seekp(offsets.at(index),std::ios::beg);
            entry.save(output);
            output.seekp(current,std::ios::beg);
            index++;
        }
        if (type == datatype_t::multi){
            amount_processed++;
            auto hashset = ultima::hashset_t();
            hashet.insert(ultima::hashLittle2("build/multicollection/housing.bin"),0);
            auto mapping = ultima::createIDTableMapping(uop, hashset, inoffsets);
            if (mapping.empty()){
                throw std::runtime_error("Error transferring housing.bin");
            }
            uop.seekg(mapping.at(0).offset+mapping.at(0).header_length,std::ios::beg);
            buffer.resize(mapping.at(0).compressed_length,0);
            uop.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
            auto entry = ultima::table_entry();
            entry.offset=static_cast<std::uint64_t>(output.tellp());
            entry.decompressed_length = mapping.at(0).decompressed_length;
            entry.compression = mapping.at(0).compression;
            entry.data_block_hash = mapping.at(0).data_block_hash ;
            output.write(reinterpret_cast<char*>(buffer.data()),buffer.size());
            output.seekp(offsets.at(index),std::ios::beg);
            entry.save(output);
        }
        std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;

    }
    catch(...){
        output.close();
        std::filesystem::remove(outputpath);
    }
}
//===============================================================================
auto idxmulMerge(const argument_t &args,datatype_t type) ->void {
    if (args.paths.size()!= 3){
        throw std::runtime_error("Invalid number of paths, format: directory idxpath mulpath") ;
    }
    auto directory = args.paths.at(0);
    auto idxpath = args.paths.at(1);
    auto mulpath = args.paths.at(2);
    auto outidxpath = idxpath;
    outidxpath.replace_extension((outidxpath.extension().string()+".bulkuo"s));
    args.writeOK(outidxpath);
    auto outmulpath = mulpath;
    outmulpath.replace_extension((outmulpath.extension().string()+".bulkuo"s));
    args.writeOK(outmulpath);
    auto data = contentsFor(directory, primaryForType(type));
    auto validids = validInContents(args, data);
    if (validids.empty()){
        throw std::runtime_error("No valid ids to merge.");
    }
    
    auto idx = std::ifstream(idxpath.string(),std::ios::binary);
    if (!idx.is_open()){
        throw std::runtime_error("Unable to open: "s + idxpath.string());
    }
    auto mappings = ultima::gatherIDXEntries(idx) ;
    idx.close();
    auto mul = std::ifstream(mulpath.string(),std::ios::binary);
    if (!mul.is_open()){
        throw std::runtime_error("Unable to open: "s + mulpath.string());
    }
    
    auto outidx = std::ofstream(outidxpath.string(),std::ios::binary);
    if (!outidx.is_open()){
        throw std::runtime_error("Unable to create: "s + outidxpath.string());
    }
    auto outmul = std::ofstream(outmulpath.string(),std::ios::binary);
    if (!outmul.is_open()){
        outidx.close();
        std::filesystem::remove(outidxpath);
        throw std::runtime_error("Unable to create: "s + outmulpath.string());
    }
    auto amount_processed = std::uint32_t(0);
    try{
        auto inoffsets = ultima::gatherIDXEntries(idx) ;
        auto number = static_cast<std::uint32_t>(std::max(inoffsets.rbegin()->first,*validids.rbegin())) + 1 ;
        auto offsets = ultima::createIDX(outidx,number);
        auto index = 0 ;
        auto buffer = std::vector<std::uint8_t>();
        auto fileflag = std::ios::binary ;
        if (type == datatype_t::multi){
            fileflag = std::ios::in ;
        }
        for (std::uint32_t id=0 ; id < number; id++){
            auto entry = ultima::idx_t();
            buffer.clear();
            entry.offset = static_cast<std::uint32_t>(outmul.tellp());
            // Is it in mu valid ids?
            if (validids.find(id)!= validids.end()){
                amount_processed++;
                auto path = data.at(id) ;
                auto input = std::ifstream(path.string(),fileflag);
                if (!input.is_open()){
                    throw std::runtime_error("Unable to open: "s+path.string());
                }
                createIDXEntry(type, id, path, input, buffer, entry);
            }
            else if (mappings.find(id) != mappings.end()){
                mul.seekg(mappings.at(id).offset,std::ios::beg);
                buffer.resize(mappings.at(id).length,0);
                mul.read(reinterpret_cast<char*>(buffer.data()),buffer.size());
                
            }
            else {
                // This doesnt exist
                entry.offset=0xFFFFFFFF;
                entry.length=0;
                entry.extra = ((type==datatype_t::art ) ?0xFFFFFFFF:0);
            }
            entry.length= static_cast<std::uint32_t>(buffer.size());
            if (!buffer.empty()){
                outmul.write(reinterpret_cast<char*>(buffer.data()),buffer.size());
            }
            outidx.seekp(offsets.at(index),std::ios::beg);
            entry.save(outidx);
            index++;
        }
        std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;
    }
    catch(...){
        outidx.close();
        outmul.close();
        std::filesystem::remove(outidxpath);
        std::filesystem::remove(outmulpath);
        throw;
    }
    
    
}
//===============================================================================
auto uopidxmulMerge(const argument_t &args,datatype_t type) ->void {
    if (args.paths.size()==2){
        uopMerge(args, type);
    }
    else if (args.paths.size()==3){
        idxmulMerge(args, type);
    }
    else {
        throw std::runtime_error("Invalid number of paths for operation.");
    }
}
//===============================================================================
auto hueMerge(const argument_t &args,datatype_t type) ->void {
    if (args.paths.size()!=2){
        throw std::runtime_error("Invalid number of paths, format: directory huemulfile");
    }
    auto directory = args.paths.at(0) ;
    auto huepath = args.paths.at(1);
    auto outpath = huepath;
    outpath.replace_extension(outpath.extension().string()+".bulkuo"s) ;
    args.writeOK(outpath);
    auto input = std::ifstream(huepath.string(),std::ios::binary);
    if (!input.is_open()){
        throw std::runtime_error("Unable to open: "s+huepath.string());
    }
    auto intotal = ultima::hueEntries(input);
    auto data = contentsFor(directory, primaryForType(type));
    auto validids = validInContents(args, data);
    auto count = static_cast<std::uint32_t>(*validids.rbegin())+1 ;
    count = std::max(count,intotal);
    auto output = std::ofstream(outpath.string(),std::ios::binary);
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s+outpath.string());
    }
    auto amount_processed = std::uint32_t(0);
    try{
        ultima::createHue(output,count);
        auto buffer = std::vector<std::uint8_t>() ;
        for (std::uint32_t id =0 ; id< count;id++){
            if (validids.find(id)!= validids.end()){
                // Ok a valid one
                amount_processed++;
                auto path = data.at(id) ;
                auto img = std::ifstream(path.string(),std::ios::binary);
                if (!img.is_open()){
                    throw std::runtime_error("Unable to open: "s+path.string());
                }
                
                auto bitmap = bitmap_t<std::uint16_t>::fromBMP(img);
                img.close();
                path = path.replace_extension(".txt") ;
                auto name = nameInFile(path);
                buffer = dataFromHue(bitmap, name);
            }
            else {
                buffer = ultima::hueData(input, id);
            }
            output.seekp(ultima::hueOffset(id),std::ios::beg);
            output.write(reinterpret_cast<char*>(buffer.data()),buffer.size());
        }
        std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;
    }
    catch(...){
        output.close();
        std::filesystem::remove(outpath);
        throw;
        
    }
};
//===============================================================================
auto infoMerge(const argument_t &args,datatype_t type) ->void {
    if (args.paths.size()!=2){
        throw std::runtime_error("Invalid number of paths, format: csvfile tiledatafile");
    }
    auto csvpath = args.paths.at(0) ;
    auto infopath = args.paths.at(1);
    auto outpath = infopath;
    outpath.replace_extension(outpath.extension().string()+".bulkuo"s) ;
    args.writeOK(outpath);
    auto info = ultima::tileinfo_t(infopath);
    auto input = std::ifstream(csvpath.string());
    auto amount_processed = updateInfo(args, input,  info) ;
    info.save(outpath);
    std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;

};
//================================================================================
auto mergeAnimationMul(const argument_t &arg,datatype_t type) ->void {
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
    auto idxoutpath = idxpath ;
    idxoutpath.replace_extension(std::filesystem::path(idxoutpath.extension().string()+".bulkuo"s));
    auto muloutpath = mulpath ;
    muloutpath.replace_extension(std::filesystem::path(muloutpath.extension().string()+".bulkuo"s));
    arg.writeOK(idxoutpath);
    arg.writeOK(muloutpath);
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
    arg.writeOK(idxpath);
    arg.writeOK(mulpath);
    auto amount_processed = 0 ;
    auto idxin = std::ifstream(idxpath.string(),std::ios::binary);
    if (!idxin.is_open()){
        throw std::runtime_error("Unable to open: "s+idxpath.string());
    }
    auto mulin = std::ifstream(mulpath.string(),std::ios::binary);
    if (!mulin.is_open()){
        throw std::runtime_error("Unable to open: "s+mulpath.string());
    }
    auto idx = std::ofstream(idxoutpath.string(),std::ios::binary) ;
    if (!idx.is_open()){
        throw std::runtime_error("Unable to create: "s +idxoutpath.string());
    }
    auto mul = std::ofstream(muloutpath.string(),std::ios::binary);
    if (!mul.is_open()){
        idx.close();
        std::filesystem::remove(idxoutpath);
        throw std::runtime_error("Unable to create: "s +muloutpath.string());
    }
    
    auto offsets = ultima::gatherIDXOffsets(idxin);
    auto numentries = std::max((*valid.rbegin())+1, static_cast<std::uint32_t>(offsets.size()));
    
    
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
            // This is not a valid id to be included,
            if (id < static_cast<std::uint32_t>(offsets.size())){
                idxin.seekg(offsets.at(id),std::ios::beg);
                auto cidx = ultima::idx_t(idxin);
                if (cidx.valid()){
                    auto buffer = std::vector<std::uint8_t>(cidx.length,0);
                    mulin.seekg(cidx.offset,std::ios::beg);
                    mulin.read(reinterpret_cast<char*>(buffer.data()),buffer.size());
                    entry.offset = static_cast<std::uint32_t>(mul.tellp());
                    entry.length = cidx.length;
                    entry.extra = cidx.extra ;
                    mul.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
                }
                else {
                    entry = cidx ;
                }
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
        entry.save(idx);
        
    }
    std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;
    
}
