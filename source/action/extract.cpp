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

#include "uophash.hpp"
#include "actutility.hpp"

#include "../artwork/texture.hpp"
#include "../artwork/art.hpp"
#include "../artwork/gump.hpp"
#include "../artwork/hue.hpp"


using namespace std::string_literals;
// Forward Declares
auto extractUOP(const argument_t &args, datatype_t type) ->void;
auto extractIdxMul(const argument_t &args, datatype_t type) ->void;
auto noExtraction(const argument_t &args, datatype_t type)->void ;
auto extractData(const argument_t &args, datatype_t type) ->void ;
auto extractHue(const argument_t &args, datatype_t type) ->void;
auto extractInfo(const argument_t &args, datatype_t type) ->void;

//==================================================================================
// Constants used
//=================================================================================

//=================================================================================
std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> extract_mapping{
    {datatype_t::art,extractData},{datatype_t::info,extractInfo},
    {datatype_t::texture,extractIdxMul},{datatype_t::sound,extractData},
    {datatype_t::gump,extractData},{datatype_t::animation,extractIdxMul},
    {datatype_t::hue,extractHue},{datatype_t::multi,extractData}
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
    auto offsets = ultima::gatherEntryOffsets(input);
    auto [hashformat,maxid] = getUOPInfoFor(type);
    auto hashset = ultima::hashset_t(hashformat, 0, maxid);
    auto mapping = ultima::createIDTableMapping(input, hashset, offsets);
    auto fileflag = std::ios::binary ;
    if (type == datatype_t::multi){
        fileflag=std::ios::in;
    }
    for (auto const &[id,entry]:mapping){
        if (args.id(id)){
            auto extension = primaryForType(type);
            auto path = args.filepath(id, directory, extension);
            args.writeOK(path);
            auto output = std::ofstream(path.string(),fileflag);
            if (!output.is_open()){
                throw std::runtime_error("Unable to create: "s + path.string());
            }
            auto buffer = ultima::readUOPData(entry, input) ;
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
                }
                default: {
                    // we shouldn't be here?
                    output.close();
                    std::filesystem::remove(path);
                }
                    
                    
            }
            
            
            
        }
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
    
    
}
//=================================================================================
auto extractHue(const argument_t &args, datatype_t type) ->void{
    
}
//=================================================================================
auto extractInfo(const argument_t &args, datatype_t type) ->void{
    
}
