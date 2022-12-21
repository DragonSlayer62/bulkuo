//Copyright © 2022 Charles Kerr. All rights reserved.

#include "catlabel.hpp"

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <vector>

#include "strutil.hpp"
#include "idlist.hpp"

using namespace std::string_literals;
//=================================================================================
auto fileparser_t::parse(const std::filesystem::path &path)->void {
    auto input = std::ifstream(path.string());
    if (!input.is_open()){
        throw std::runtime_error("Unable to open: "s + path.string());
    }
    auto buffer = std::vector<char>(4098,0);
    while (input.good() && !input.eof()){
        input.getline(buffer.data(),4097);
        if (input.gcount()>0){
            buffer.at(input.gcount()) = 0 ;
            std::string line = buffer.data();
            line = strutil::trim(strutil::strip(line,"//"));
            auto [left,right] = strutil::split(line,"=");
            // We only care if both are not empty
            if (!left.empty() && !right.empty()){
                lineentry(left, right);
            }
        }
    }
}
const std::string label_t::empty_string ="";

//=================================================================================
auto label_t::lineentry(const std::string &left, const std::string &right) ->void {
    // Left is id, right is string
    auto id = static_cast<std::uint32_t>(std::stoul(left,nullptr,0)) ;
    labels.insert_or_assign(id,right);
}

//=================================================================================
auto label_t::label(std::uint32_t id) const ->const std::string& {
    auto iter = labels.find(id) ;
    if (iter != labels.end()){
        return iter->second;
    }
    else {
        return empty_string;
    }
}
//=================================================================================
label_t::label_t(const std::filesystem::path &path){
    if (!path.empty()){
        parse(path);
    }
}
//=================================================================================
auto category_t::lineentry(const std::string &left, const std::string &right) ->void {
    // Left is path, right is list
    auto ids = processList(right);
    auto path = std::filesystem::path(left) ;
    for (const auto &id:ids){
        categories.insert_or_assign(id,path);
    }
}
//=================================================================================
auto category_t::category(std::uint32_t id,const std::filesystem::path &directory) const ->std::filesystem::path {
    auto iter = categories.find(id) ;
    if (iter != categories.end()){
        auto path = directory/iter->second;
        if (!std::filesystem::exists(path)){
            std::filesystem::create_directories(path);
        }
        return path ;
    }
    else {
        return directory;
    }
}
//=================================================================================
category_t::category_t(const std::filesystem::path &path){
    if (!path.empty()){
        parse(path);
    }
}

