//Copyright © 2022 Charles Kerr. All rights reserved.

#include "idlist.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <algorithm>

#include "strutil.hpp"
using namespace std::string_literals;

//=================================================================================
auto processList(const std::string &line) ->std::set<std::uint32_t> {
    auto rvalue = std::set<std::uint32_t>();
    auto values = strutil::parse(line,",");
    for (const auto &entry:values){
        if (!entry.empty()){
            auto [first,second] = strutil::split(entry,"-") ;
            auto start = static_cast<std::uint32_t>(std::stoul(first,nullptr,0)) ;
            
            if (second.empty()){
                rvalue.insert(start) ;
            }
            else {
                auto end = static_cast<std::uint32_t>(std::stoul(second,nullptr,0)) ;
                for (auto j=start;j<=end;j++){
                    rvalue.insert(j);
                    
                }
            }
        }
    }
    return rvalue;
}
//=================================================================================
auto processList(const std::filesystem::path &path) ->std::set<std::uint32_t> {
    auto input = std::ifstream(path.string(),std::ios::binary);
    if (!input.is_open()){
        throw std::runtime_error("Unable to open: "s + path.string());
    }
    auto buffer = std::vector<char>(4099,0);
    auto rvalue = std::set<std::uint32_t>() ;
    while (input.good() && !input.eof()){
        input.getline(buffer.data(),4098);
        if ((input.gcount()>0) && input.good()){
            buffer.at(input.gcount())=0 ;
            std::string line = buffer.data();
            line = strutil::trim(strutil::strip(line,"//"));
            auto linedata = processList(line) ;
            for (const auto &id:linedata){
                rvalue.insert(id);
            }
        }
    }
    return rvalue ;
}
//=================================================================================
// Convert a set to a "list" format
auto setToList(const std::set<std::uint32_t> &values,bool use_hex) ->std::string {
    auto output = std::stringstream() ;
    if (use_hex){
        output <<std::hex<<std::showbase ;
        
    }
    auto iter = values.begin();
    auto start = std::uint32_t(std::numeric_limits<std::uint32_t>::max());
    auto last = std::uint32_t(std::numeric_limits<std::uint32_t>::max());
    while (iter != values.end()){
        
        if (start ==std::numeric_limits<std::uint32_t>::max()){
            output <<*iter ;
            
            start = *iter ;
            last = *iter ;
        }
        
        if ((*iter == last) || ( *iter == (last+1) ) ){
            last = *iter ;
        }
        else {
            if (start != last){
                output << "-"<<last ;
            }
            output << ","<<*iter ;
            start = *iter ;
            last = *iter ;
        }
        
        iter++;
    }
    if (last != start){
        output <<"-"<<last ;
    }
    return output.str();
}
