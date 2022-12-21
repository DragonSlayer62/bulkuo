//Copyright © 2022 Charles Kerr. All rights reserved.

#include "names.hpp"

#include <iostream>
#include <stdexcept>
#include <fstream>

#include "strutil.hpp"
#include "uophash.hpp"

#include "../uodata/tileinfo.hpp"
#include "../uodata/multicollection.hpp"
#include "../uodata/hueaccess.hpp"
#include "../uodata/idx.hpp"
#include "../uodata/uop.hpp"
#include "../uodata/hash.hpp"
#include "../uodata/uowave.hpp"
using namespace std::string_literals;

//=================================================================================
auto infoName(const argument_t &arg,datatype_t type) ->void ;
auto multiName(const argument_t &arg,datatype_t type) ->void ;
auto hueName(const argument_t &arg,datatype_t type) ->void ;
auto soundName(const argument_t &arg,datatype_t type) ->void ;
//=================================================================================
auto noNames(const argument_t &args,datatype_t type) ->void {
    auto name = nameForDatatype(type);
    throw std::runtime_error("Action: 'names'is not supported for type: "s+name);
}
//==================================================================================
// Constants used
//=================================================================================
//=================================================================================
std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> names_mapping{
    {datatype_t::art,noNames},{datatype_t::info,infoName},
    {datatype_t::texture,noNames},{datatype_t::sound,soundName},
    {datatype_t::gump,noNames},{datatype_t::animation,noNames},
    {datatype_t::hue,hueName},{datatype_t::multi,multiName}
};

//=================================================================================
auto santizeName(const std::string& name) -> std::string {
    auto temp = strutil::trim(name);
    if (!temp.empty()) {
        auto pos = temp.find(">");
        while (pos != std::string::npos) {
            temp.replace(pos, 1, "-");
            pos = temp.find(">");
        }
        pos = temp.find("?");
        while (pos != std::string::npos) {
            temp.replace(pos, 1, "");
            pos = temp.find("?");
        }
        pos = temp.find("/");
        while (pos != std::string::npos) {
            temp.replace(pos, 1, "-");
            pos = temp.find("/");
        }
        pos = temp.find("\\");
        while (pos != std::string::npos) {
            temp.replace(pos, 1, "-");
            pos = temp.find("\\");
        }
        if (temp.find_first_not_of("-") == std::string::npos) {
            temp = "";
        }
        return temp;
    }

}

//=================================================================================
auto infoName(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()!= 2){
        throw std::runtime_error("Incorrect number of paths, format is: tiledatapath namefilepath");
    }
    auto infopath = arg.paths.at(0);
    auto outputpath = arg.paths.at(1) ;
    arg.writeOK(outputpath) ;

    auto info = ultima::tileinfo_t(infopath);
    
    auto output = std::ofstream(outputpath.string());
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s + outputpath.string());
    }
    if (arg.use_hex){
        output <<std::hex<<std::showbase;
    }
    for (std::uint32_t id= 0 ; id < std::uint32_t(0xFFFF + 0x4000);id++){
        auto name = std::string();
        if (arg.id(id)){
            if (id<0x4000){
                name = info.land(id).name;
            }
            else{
                name = info.item(id-0x4000).name;
            }
            name = santizeName(name);
            if (!name.empty()){
                output << id << " = "<<name<<"\n";
            }
        }
    }
}
//=====================================================================================
auto multiName(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()!= 1){
        throw std::runtime_error("Incorrect number of paths, format is: namefilepath");
    }
    auto outputpath = arg.paths.at(0) ;
    arg.writeOK(outputpath) ;
    auto output = std::ofstream(outputpath.string());
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s + outputpath.string());
    }
    if (arg.use_hex){
        output <<std::hex<<std::showbase;
    }

    for (const auto &[id,name]:ultima::multi_entry_t::names){
        output <<id <<" = " << santizeName(name) <<"\n";
    }
}
//=====================================================================================
auto hueName(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()!= 2){
        throw std::runtime_error("Incorrect number of paths, format is: huefilepath namefilepath");
    }
    auto huepath = arg.paths.at(0);
    auto outputpath = arg.paths.at(1) ;
    arg.writeOK(outputpath) ;
    auto hue = std::ifstream(huepath.string(),std::ios::binary) ;
    if (!hue.is_open()){
        throw std::runtime_error("Unable to open: "s + huepath.string());
    }
    auto maxid = ultima::hueEntries(hue);
    
    auto output = std::ofstream(outputpath.string());
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s + outputpath.string());
    }
    if (arg.use_hex){
        output <<std::hex<<std::showbase;
    }
    auto buffer = std::vector<std::uint8_t>(ultima::hue_entry_size,0);
    for (std::uint32_t id=0 ; id<maxid;id++){
        if (arg.id(id)){
            auto name = ultima::nameForHue(hue,id);
            name = santizeName(name);
            if(!name.empty()){
                output <<id <<" = " << name << "\n";
            }
        }
    }
}
//=====================================================================================
auto soundIDXName(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()!= 3){
        throw std::runtime_error("Incorrect number of paths, format is: soundidxpath soundmulpath namefilepath");
    }
    auto idxpath = arg.paths.at(0);
    auto mulpath = arg.paths.at(1);
    auto outputpath = arg.paths.at(2) ;
    arg.writeOK(outputpath) ;
    auto idx = std::ifstream(idxpath.string(),std::ios::binary) ;
    if (!idx.is_open()){
        throw std::runtime_error("Unable to open: "s + idxpath.string());
    }
    auto mul = std::ifstream(mulpath.string(),std::ios::binary) ;
    if (!mul.is_open()){
        throw std::runtime_error("Unable to open: "s + mulpath.string());
    }
    auto output = std::ofstream(outputpath.string());
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s + outputpath.string());
    }
    if (arg.use_hex){
        output <<std::hex<<std::showbase;
    }
    auto entries = ultima::gatherIDXEntries(idx);
    auto buffer = std::vector<char>(17,0) ;
    for (const auto &[id,entry]:entries){
        if (arg.id(id)){
            mul.seekg(entry.offset,std::ios::beg);
            mul.read(buffer.data(),16);
            auto name = ultima::nameForSound(buffer);
            if (!name.empty()){
                output <<id <<" = " << name <<"\n";
            }
        }
    }
}

//=====================================================================================
auto soundUOPName(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()!= 2){
        throw std::runtime_error("Incorrect number of paths, format is: sounduoppath  namefilepath");
    }
    auto uoppath = arg.paths.at(0);
    auto outputpath = arg.paths.at(1) ;
    arg.writeOK(outputpath) ;
    auto uop = std::ifstream(uoppath.string(),std::ios::binary) ;
    if (!uop.is_open()){
        throw std::runtime_error("Unable to open: "s + uoppath.string());
    }
    auto output = std::ofstream(outputpath.string());
    if (!output.is_open()){
        throw std::runtime_error("Unable to create: "s + outputpath.string());
    }
    if (arg.use_hex){
        output <<std::hex<<std::showbase;
    }
    auto [hashformat,maxid] = getUOPInfoFor(type);
    auto hashset = ultima::hashset_t(hashformat, 0, maxid);
    auto offsets = ultima::gatherEntryOffsets(uop);
    auto mapping = ultima::createIDTableMapping(uop, hashset, offsets);
    auto buffer = std::vector<char>(17,0) ;
    for (const auto &[id,entry]:mapping){
        if (arg.id(id)){
            uop.seekg(entry.offset+entry.header_length,std::ios::beg);
            uop.read(buffer.data(),16);
            auto name = ultima::nameForSound(buffer);
            if (!name.empty()){
                output <<id <<" = " << name <<"\n";
            }
        }
    }
}
//=============================================================================================
auto soundName(const argument_t &arg,datatype_t type) ->void{
    if (arg.paths.size()==3){
        soundIDXName(arg, type);
    }
    else if (arg.paths.size()==2){
        soundUOPName(arg, type);
    }
    else {
        throw std::runtime_error("Incorrect number of paths, format is: sounduoppath  namefilepath \nOr: soundidxpath soundmulpath namefilepath");

    }
}
