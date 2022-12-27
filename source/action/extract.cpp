//Copyright © 2022 Charles Kerr. All rights reserved.

#include "extract.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <map>

#include "strutil.hpp"

#include "../uodata/uop.hpp"
#include "../uodata/hash.hpp"
#include "../uodata/uowave.hpp"
#include "../uodata/multicollection.hpp"
#include "../uodata/hueaccess.hpp"
#include "../uodata/tileinfo.hpp"

#include "uophash.hpp"
#include "actutility.hpp"

#include "../artwork/texture.hpp"
#include "../artwork/art.hpp"
#include "../artwork/gump.hpp"
#include "../artwork/hue.hpp"
#include "../artwork/light.hpp"
#include "../artwork/animation.hpp"


using namespace std::string_literals;
// Forward Declares
auto extractUOP(const argument_t &args, datatype_t type) ->void;
auto extractIdxMul(const argument_t &args, datatype_t type) ->void;
auto noExtraction(const argument_t &args, datatype_t type)->void ;
auto extractData(const argument_t &args, datatype_t type) ->void ;
auto extractHue(const argument_t &args, datatype_t type) ->void;
auto extractInfo(const argument_t &args, datatype_t type) ->void;
auto extractAnimation(const argument_t &args, datatype_t type) ->void;
//==================================================================================
// Constants used
//=================================================================================

//=================================================================================
std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> extract_mapping{
    {datatype_t::art,extractData},{datatype_t::info,extractInfo},
    {datatype_t::texture,extractIdxMul},{datatype_t::sound,extractData},
    {datatype_t::gump,extractData},{datatype_t::animation,extractAnimation},
    {datatype_t::hue,extractHue},{datatype_t::multi,extractData},
    {datatype_t::light,extractIdxMul}
};

//=================================================================================
auto noExtraction(const argument_t &args, datatype_t type)->void {
    auto name = nameForDatatype(type);
    std::cout <<"'"<<name<<"'"<<" is not extractable yet.\n";
}
//=================================================================================
auto extractData(const argument_t &args, datatype_t type) ->void {
    if (args.paths.size() == 2){
        extractUOP(args, type);
    }
    else if (args.paths.size()==3){
        extractIdxMul(args, type);
    }
    else {
        throw std::runtime_error("For uop, paths required are: uoppath directory\nFor idx/mul, paths arequired are: idxpath mulpath directory");
    }
}

//=================================================================================
auto extractUOP(const argument_t &args, datatype_t type) ->void{
    if (args.paths.size() != 2){
        throw std::runtime_error("For uop, paths required are: uoppath directory.");
    }
    auto uoppath = args.paths.at(0);
    auto directory = args.paths.at(1);
    
    auto input = std::ifstream(uoppath.string(),std::ios::binary);
    if (!input.is_open()){
        throw std::runtime_error("Unable to open: "s + uoppath.string());
    }
    if (!ultima::validUOP(input)){
        throw std::runtime_error("Invalid uop format: "s + uoppath.string());
        
    }
    auto offsets = ultima::gatherEntryOffsets(input);
    auto [hashformat,maxid] = getUOPInfoFor(type);
    auto hashset = ultima::hashset_t(hashformat, 0, maxid);
    auto mapping = ultima::createIDTableMapping(input, hashset, offsets);
    auto fileflag = std::ios::binary ;
    if (type == datatype_t::multi){
        fileflag=std::ios::out;
    }
    auto amount_processed = std::uint32_t(0) ;
    try{
        for (auto const &[id,entry]:mapping){
            if (args.id(id)){
                if (type!= datatype_t::animation){
                    auto extension = primaryForType(type);
                    auto path = args.filepath(id, directory, extension);
                    args.writeOK(path);
                    auto output = std::ofstream(path.string(),fileflag);
                    if (!output.is_open()){
                        throw std::runtime_error("Unable to create: "s + path.string());
                    }
                    auto buffer = ultima::readUOPData(entry, input) ;
                    amount_processed++;
                    switch(type){
                        case datatype_t::gump:{
                            auto bitmap = bitmap_t<std::uint16_t>();
                            bitmap = bitmapForGump(buffer);
                            bitmap.saveToBMP(output,args.colorsize);
                            break;
                        }
                        case datatype_t::art:{
                            auto bitmap = bitmap_t<std::uint16_t>();
                            if (id <0x4000){
                                bitmap = bitmapForTerrain(buffer);
                            }
                            else {
                                bitmap = bitmapForItem(buffer);
                            }
                            bitmap.saveToBMP(output,args.colorsize);
                            break;
                        }
                        case datatype_t::sound:{
                            auto secondary = secondaryForType(type);
                            auto secondpath=args.filepath(id,directory,secondary);
                            args.writeOK(secondpath);
                            
                            auto wav = ultima::uowave_t();
                            auto name = wav.loadUO(buffer.data(), buffer.size()) ;
                            auto soutput = std::ofstream(secondpath.string());
                            if (!soutput.is_open()){
                                output.close();
                                std::filesystem::remove(path) ;
                                throw std::runtime_error("Unable to create: "s + secondpath.string());
                            }
                            soutput<<name<<std::endl;
                            soutput.close();
                            wav.save(output);
                            break;
                        }
                        case datatype_t::multi: {
                            auto entry = ultima::multi_entry_t(buffer,true);
                            entry.description(output, args.use_hex);
                            output.close();
                            break;
                        }
                        case datatype_t::animation: {
                            break;
                        }
                        default: {
                            // we shouldn't be here?
                            output.close();
                            std::filesystem::remove(path);
                            throw std::runtime_error("UOP extract not supported for data type.");
                        }
                    }
                }
                else {
                    
                }
            }
        }
        // We are almost done.  We need to check for multi, to get housing bin!
        if (type == datatype_t::multi){
            auto hashset = ultima::hashset_t();
            hashset.insert(ultima::hashLittle2("build/multicollection/housing.bin"), 0) ;
            auto mapping = ultima::createIDTableMapping(input, hashset, offsets);
            for (const auto &[id,entry]:mapping){
                auto buffer = ultima::readUOPData(entry, input);
                auto path = directory / std::filesystem::path("housing.bin");
                args.writeOK(path);
                auto output = std::ofstream(path.string(),std::ios::binary);
                if (!output.is_open()){
                    throw std::runtime_error("Unable to create: "s + path.string());
                }
                amount_processed++;
                output.write(reinterpret_cast<char*>(buffer.data()),buffer.size());
                output.close();
            }
            
        }
        std::cout<<"Processed " <<amount_processed<<" entries."<<std::endl;
    }
    catch(...){
        throw;
    }
}
//=================================================================================
auto extractIdxMul(const argument_t &args, datatype_t type) ->void{
    if (args.paths.size() != 3){
        throw std::runtime_error("For idx/mul, paths required are: idxpath mulpath directory.");
    }
    auto idxpath = args.paths.at(0);
    auto mulpath = args.paths.at(1);
    auto directory = args.paths.at(2);
    auto amount_processed = std::uint32_t(0);
    auto idx = std::ifstream(idxpath.string(),std::ios::binary);
    if (!idx.is_open()){
        throw std::runtime_error("Unable to open: "s + idxpath.string());
    }
    auto mul = std::ifstream(mulpath.string(),std::ios::binary);
    if (!mul.is_open()){
        throw std::runtime_error("Unable to open: "s +mulpath.string());
    }
    auto fileflag = std::ios::binary ;
    if (type == datatype_t::multi){
        fileflag=std::ios::out;
    }
    auto entries = ultima::gatherIDXEntries(idx);
    try{
        for (const auto &[id,entry]:entries){
            auto bitmap = bitmap_t<std::uint16_t>();
            if (args.id(id)){
                auto path = args.filepath(id, directory, primaryForType(type));
                args.writeOK(path);
                auto output = std::ofstream(path.string(),fileflag);
                if (!output.is_open()){
                    throw std::runtime_error("Unable to create: "s + path.string());
                }
                
                auto buffer = ultima::readMulData(mul, entry);
                amount_processed++;
                switch(type){
                    case datatype_t::gump: {
                        auto temp = std::vector<std::uint8_t>(8,0);
                        std::copy(reinterpret_cast<const std::uint8_t*>(&entry.extra),reinterpret_cast<const std::uint8_t*>(&entry.extra)+2,temp.begin()+4);
                        std::copy(reinterpret_cast<const std::uint8_t*>(&entry.extra)+2,reinterpret_cast<const std::uint8_t*>(&entry.extra)+4,temp.begin());
                        temp.insert(temp.end(),buffer.begin(),buffer.end());
                        bitmap = bitmapForGump(temp);
                        bitmap.saveToBMP(output,args.colorsize);
                        break;
                    }
                    case datatype_t::art: {
                        if (id <0x4000){
                            bitmap = bitmapForTerrain(buffer);
                        }
                        else {
                            bitmap = bitmapForItem(buffer);
                        }
                        bitmap.saveToBMP(output,args.colorsize);
                        break;
                    }
                    case datatype_t::texture: {
                        bitmap = bitmapForTexture(buffer);
                        bitmap.saveToBMP(output,args.colorsize);
                        break;
                    }
                    case datatype_t::sound: {
                        auto secondary = secondaryForType(type);
                        auto secondpath=args.filepath(id,directory,secondary);
                        args.writeOK(secondpath);
                        
                        auto wav = ultima::uowave_t();
                        auto name = wav.loadUO(buffer.data(), buffer.size()) ;
                        auto soutput = std::ofstream(secondpath.string());
                        if (!soutput.is_open()){
                            output.close();
                            std::filesystem::remove(path) ;
                            throw std::runtime_error("Unable to create: "s + secondpath.string());
                        }
                        soutput<<name<<std::endl;
                        soutput.close();
                        wav.save(output);
                        break;
                    }
                    case datatype_t::multi: {
                        auto entry = ultima::multi_entry_t(buffer,false);
                        entry.description(output, args.use_hex);
                        output.close();
                        
                        break;
                    }
                    case datatype_t::light:{
                        auto width = entry.extra&0xFFFF ;
                        auto height = (entry.extra>>16)&0xFFFF;
                        bitmap = bitmapForLight(width, height, buffer);
                        bitmap.saveToBMP(output,args.colorsize);
                        break;
                    }
                    default:{
                        output.close();
                        std::filesystem::remove(path);
                        throw("IDX/MUL extract not supported for this type.");
                        break;
                        
                    }
                }
            }
        }
        std::cout <<"Processed " << amount_processed<< " entries"<<std::endl;
    }
    catch(...){
        throw;
    }
}
//=================================================================================
auto extractHue(const argument_t &args, datatype_t type) ->void{
    if (args.paths.size() != 2){
        throw std::runtime_error("For hues, paths required are: uomulpath directory.");
    }
    auto inputpath = args.paths.at(0);
    auto directory = args.paths.at(1);
    
    auto input = std::ifstream(inputpath.string(),std::ios::binary);
    if (!input.is_open()){
        throw std::runtime_error("Unable to open: "s + inputpath.string());
    }
    auto maxid = ultima::hueEntries(input) ;
    auto amount_processed = std::uint32_t(0);
    for (std::uint32_t id=0 ; id <maxid;id++) {
        if (args.id(id) ){
        
            auto buffer = ultima::hueData(input, id);
            if (!hueBlank(buffer)){
                auto path = args.filepath(id, directory, ".bmp");
                auto secondary = args.filepath(id, directory, ".txt");
                args.writeOK(path);
                args.writeOK(secondary);
                
                auto [bitmap,name] = hueFromData(buffer,std::get<0>(args.huesize),std::get<1>(args.huesize));
                auto output = std::ofstream(path.string(),std::ios::binary);
                if (!output.is_open()){
                    throw std::runtime_error("Unable to create: "s + path.string());
                }
                auto sec_output = std::ofstream(secondary.string());
                if (!sec_output.is_open()){
                    throw std::runtime_error("Unable to create: "s + secondary.string());
                }
                amount_processed++;
                bitmap.saveToBMP(output,args.colorsize);
                sec_output<<name<<std::endl;
                sec_output.close();
            }
        }
    }
    std::cout <<"Processed " << amount_processed<<" entries."<<std::endl;
    
}
//=================================================================================
auto extractInfo(const argument_t &args, datatype_t type) ->void{
    if (args.paths.size() != 2){
        throw std::runtime_error("For info, paths required are: tiledatamulpath outputfile.");
    }
    auto inputpath = args.paths.at(0);
    auto outputpath = args.paths.at(1);
    auto info = ultima::tileinfo_t(inputpath) ;
    args.writeOK(outputpath);
    auto output= std::ofstream(outputpath.string());
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s + outputpath.string());
    }
    output << "tileid,"<<ultima::tilebase_t::header()<<std::endl;
    auto amount_processed = std::uint32_t(0);
    for (std::uint32_t id = 0 ; id < 0x10000;id++){
        if (args.id(id)){
            amount_processed++;
            if (args.use_hex){
                output <<std::hex<<std::showbase<<id<<std::dec<<std::noshowbase<<",";
            }
            else {
                output <<id<<",";
            }
            if (id<0x4000){
                
                output <<info.land(id).description(args.use_hex)<<"\n";
            }
            else {
                output <<info.item(id-0x4000).description(args.use_hex)<<"\n";
            }
        }
    }
    std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;
}

//==================================================================================
auto extractAnimation(const argument_t &args, datatype_t type) ->void{
    if (args.paths.size() != 4){
        throw std::runtime_error("Invalid # of paths, format: idxfile mulfile bmp_csv_directory replaceable_directory ");
    }
    auto idxpath = args.paths.at(0);
    auto mulpath = args.paths.at(1);
    auto directory = args.paths.at(2);
    auto secondary_directory = args.paths.at(3);
    auto amount_processed = 0 ;
    auto idx = std::ifstream(idxpath.string(),std::ios::binary) ;
    if (!idx.is_open()){
        throw std::runtime_error("Unable to open: "s+idxpath.string());
    }
    auto mul = std::ifstream(mulpath.string(),std::ios::binary) ;
    if (!mul.is_open()){
        throw std::runtime_error("Unable to open: "s+mulpath.string());
    }
    idx.seekg(0,std::ios::end) ;
    auto numentries = static_cast<std::uint32_t>(idx.tellg()/12) ;
    idx.seekg(0,std::ios::beg) ;
    for (std::uint32_t id = 0 ; id < numentries;id++){
        auto entry = ultima::idx_t(idx);
        // First, do we care about this id?
        if (args.id(id)){
            // We do
            // It can be one of three, it can "replaceable", or a picture, or empty.
            if ((entry.offset == 0) && (entry.length == 0) && (entry.extra == 0) ){
                // this is a swapable!
                amount_processed++;
                auto path = args.filepath(id, secondary_directory, ".swapped");
                args.writeOK(path);
                auto output = std::ofstream(path.string());
                if (!output.is_open()){
                    throw std::runtime_error("Unable to create: "s+path.string());
                }
            }
            else if ((entry.offset != 0xFFFFFFFF) && (entry.length > 0) && (entry.extra != 0xFFFFFFFF)){
                amount_processed++ ;
                mul.seekg(entry.offset,std::ios::beg);
                auto buffer = std::vector<std::uint8_t>(entry.length,0);
                mul.read(reinterpret_cast<char*>(buffer.data()),buffer.size());
                auto animation = animation_t(buffer);
                auto path = args.filepath(id, directory, ".csv");
                args.writeOK(path);
                auto output = std::ofstream(path.string());
                output << animation.description();
                output.close() ;
                auto [first,label] = strutil::split(path.stem().string(),"-");
                auto framenum = 0 ;
                for (const auto &frame:animation.frames){
                    if (!frame.image.empty()){
                        auto file =  first+"."s+std::to_string(framenum) + (label.empty()?""s:("-"s+label)) + ".bmp"s;
                        path.replace_filename(std::filesystem::path(file));
                        args.writeOK(path);
                        auto output = std::ofstream(path.string(),std::ios::binary);
                        if (!output.is_open()){
                            throw std::runtime_error("Unable to create: "s+path.string());
                        }
                        frame.image.saveToBMP(output,args.colorsize);
                     }
                }

            }
        }
        
    }
    std::cout <<"Processed "<<amount_processed<<" entries."<<std::endl;
}

