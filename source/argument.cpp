//Copyright © 2022 Charles Kerr. All rights reserved.

#include "argument.hpp"

#include <iostream>
#include <stdexcept>

#include "idlist.hpp"
#include "strutil.hpp"
using namespace std::string_literals;

//=================================================================================

//=================================================================================
argument_t::argument_t(int argc, const char* argv[]):use_hex(false),colorsize(16),overwrite(false),version(false),help(false),action(action_t::unknown),type(datatype_t::unknown),huesize{10,10}{
    for (auto j=1 ; j<argc;j++){
        auto parameter = std::string(argv[j]);
        auto pos = parameter.find("--");
        if (pos == 0 ){
            // This is a flag
            if (parameter.size()<=2){
                throw std::runtime_error("Invalid parameter on command line: "s + parameter);
            }
            parameter = parameter.substr(2) ;
            auto [flag,value] = strutil::split(parameter,"=");
            flag_and_value.push_back(std::make_pair(strutil::lower(flag), value)) ;
        }
        else {
            // This is a parameter
            paths.push_back(std::filesystem::path(parameter));
        }
    }
    processFlags();
}
//=================================================================================
auto argument_t::processFlags()->void{
    for (const auto &[flag,value]:flag_and_value){
        if (flag == "hex"){
            use_hex = true ;
        }
        else if (flag == "bmp24"){
            colorsize = 24 ;
        }
        else if (flag == "help"){
            help = true ;
        }
        else if (flag == "version"){
            version = true ;
        }
        else if (flag == "id"){
            processID(value);
        }
        else if (flag == "label"){
            labels = label_t(std::filesystem::path(value));
        }
        else if (flag == "category"){
            categories = category_t(std::filesystem::path(value));
        }
        else if (flag == "overwrite"){
            overwrite = true ;
        }
        else {
            auto temp = actionForName(flag) ;
            if (temp != action_t::unknown){
                if (action != action_t::unknown){
                    // We all ready set the action!
                    throw std::runtime_error("Only one action may be requested.");
                }
                action = temp;
                action_value = value ;
            }
            else {
                auto dtemp = datatypeForName(flag) ;
                if (dtemp == datatype_t::unknown){
                    // Ok, so we have a flag we no nothing about
                    throw std::runtime_error("Unrecognized flag: --"s+flag+(value.empty()?""s:"="s+value));
                }
                type = dtemp ;
                if (type == datatype_t::hue){
                    if (!value.empty()){
                        auto [swidth,sheight] = strutil::split(value,",");
                        auto width = std::get<0>(huesize) ;
                        auto height = std::get<1>(huesize);
                        if (!swidth.empty()){
                            width = std::stoi(swidth) ;
                        }
                        if (!sheight.empty()){
                            height = std::stoi(sheight);
                        }
                        huesize = std::make_pair(width,height) ;
                    }
                }
            }
        }
    }
}
//=================================================================================
auto argument_t::processID(const std::string &line)->void{
    valid_ids.clear();
    if (!line.empty()){
        auto [type,rest] = strutil::split(line,":");
        type = strutil::lower(type) ;
        if (type == "l"){
            // It is a list!
            valid_ids = processList(rest);
        }
        else if (type == "f"){
            // it is a file
            auto path = std::filesystem::path(rest);
            valid_ids = processList(path);
        }
        else {
            throw std::runtime_error("Invalid value for --id, must specify a list (--id=l:listvalues), or filepath (--id=f:filepath)");
        }
    }
}

//=================================================================================
auto argument_t::id(std::uint32_t id) const ->bool {
    if (valid_ids.empty()){
        return true ;
    }
    auto iter = valid_ids.find(id) ;
    if (iter != valid_ids.end()){
        return true ;
    }
    return false ;
}
//=================================================================================
auto argument_t::label(std::uint32_t id) const ->const std::string& {
    return labels.label(id);
}
//=================================================================================
auto argument_t::locationFor(std::uint32_t id, const std::filesystem::path &directory) const ->std::filesystem::path {
    return categories.category(id, directory);
}
//=================================================================================
auto argument_t::filepath(std::uint32_t id, const std::filesystem::path &directory, const std::string &extension) const ->std::filesystem::path{
    // First convert the id to string
    auto format = std::string("%05u");
    if (use_hex){
        format = std::string("0x%05X");
    }
    auto name = strutil::format(format,id);
    auto extra = label(id) ;
    if (!extra.empty()){
        name = name +"-"s + extra ;
    }
    name = name + extension ;
    return locationFor(id, directory) / std::filesystem::path(name);
}
//=================================================================================
auto argument_t::writeOK(const std::filesystem::path &path,bool exception) const ->bool{
    auto rvalue = (!std::filesystem::exists(path)) || this->overwrite ;
    if (!rvalue && exception){
        throw std::runtime_error("Creation would overwrite file without --overwrite: "s+path.string());
    }
    return rvalue ;
}
