//Copyright © 2022 Charles Kerr. All rights reserved.

#include "exist.hpp"

#include <iostream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <filesystem>
#include <fstream>

#include "../idlist.hpp"

#include "../uodata/uop.hpp"
#include "../uodata/idx.hpp"
#include "../uodata/hueaccess.hpp"

#include "../artwork/hue.hpp"

#include "uophash.hpp"

using namespace std::string_literals;

//=================================================================================
// Forward Declares
//=================================================================================
//=================================================================================
auto uopExist(const argument_t &arg,datatype_t type) ->void ;
auto idxmulExist(const argument_t &arg,datatype_t type) ->void ;
auto genericExist(const argument_t &arg,datatype_t type) ->void ;
auto hueExist(const argument_t &arg,datatype_t type) ->void ;
auto noExist(const argument_t &args,datatype_t type) ->void;


//==================================================================================
// Constants used
//=================================================================================

//=================================================================================
std::map<datatype_t,std::function<void(const argument_t&,datatype_t)>> exist_mapping{
    {datatype_t::art,genericExist},{datatype_t::info,noExist},
    {datatype_t::texture,idxmulExist},{datatype_t::sound,genericExist},
    {datatype_t::gump,genericExist},{datatype_t::hue,hueExist},
    {datatype_t::multi,genericExist},{datatype_t::animation,idxmulExist},
    {datatype_t::light,idxmulExist}
};



//=================================================================================
auto noExist(const argument_t &args,datatype_t type) ->void {
    auto name = nameForDatatype(type);
    throw std::runtime_error("Action: 'exist'is not supported for type: "s+name);
}

//=================================================================================
auto uopExist(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size() != 1){
        throw std::runtime_error("Invalid number of paths, format is: uoppath") ;
    }
    auto uoppath = arg.paths.at(0) ;
    auto input = std::ifstream(uoppath.string(),std::ios::binary);
    if (!input.is_open()){
        throw std::runtime_error("Unable to open: "s+uoppath.string());
    }
    if (!ultima::validUOP(input)){
        throw std::runtime_error("Not recognized as a valid uop: "s+uoppath.string());
    }
    auto [hashformat,maxid] = getUOPInfoFor(type);
    auto hashset = ultima::hashset_t(hashformat, 0, maxid);
    auto offsets = ultima::gatherEntryOffsets(input);
    auto mapping = ultima::createIDTableMapping(input, hashset, offsets);
    auto ids = std::set<std::uint32_t>() ;
    for (const auto &[id,entry]:mapping){
        if (type == datatype_t::gump){
            // We do this check, there are gump entries with just width/height, no data
            if (entry.compressed_length>8){
                if (arg.id(id)) {
                    ids.insert(id) ;
                }
            }
        }
        else {
            if (arg.id(id)) {
                ids.insert(id) ;
            }
        }
    }
    std::cout << setToList(ids, arg.use_hex)<<std::endl;
}
//=================================================================================
auto idxmulExist(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size() != 2){
        throw std::runtime_error("Invalid number of paths, format is: idxpath mulpath") ;
    }
    auto idxpath = arg.paths.at(0) ;
    auto mulpath = arg.paths.at(1) ;
    
    auto idx = std::ifstream(idxpath.string(),std::ios::binary);
    if (!idx.is_open()){
        throw std::runtime_error("Unable to open: "s+idxpath.string());
    }

    auto entries = ultima::gatherIDXEntries(idx);
    auto ids = std::set<std::uint32_t>() ;
    for (const auto &[id,entry]:entries){
        if (arg.id(id)) {
            ids.insert(id) ;
        }
    }
    std::cout << setToList(ids, arg.use_hex)<<std::endl;
}
//=================================================================================
auto genericExist(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()==1){
        uopExist(arg, type);
    }
    else if (arg.paths.size()==2){
        idxmulExist(arg, type);
    }
    else {
        throw std::runtime_error("Invalid number of path entries.\nEither uop: uoppath\nor idx/mul: idxpath mulpath"s);
    }
}
//=================================================================================
auto hueExist(const argument_t &arg,datatype_t type) ->void {
    if (arg.paths.size()!= 1){
        throw std::runtime_error("Invalid number of path entries. Format is: huepath"s);
    }
    auto ids = std::set<std::uint32_t>() ;
    if (arg.id(0)){
        ids.insert(0) ; // id 0 is always used
    }
    auto huepath = arg.paths.at(0) ;
    auto input = std::ifstream(huepath.string(),std::ios::binary);
    if (!input.is_open()){
        throw std::runtime_error("Unable to open: "s + huepath.string());
    }
    auto entries = ultima::hueEntries(input) ;
    for (std::uint32_t id = 0 ; id <entries;id++){
        auto buffer = ultima::hueData(input, id);
        if (!hueBlank(buffer)){
            if (arg.id(id)) {
                ids.insert(id) ;
            }
        }
    }
    std::cout << setToList(ids, arg.use_hex)<<std::endl;
}



