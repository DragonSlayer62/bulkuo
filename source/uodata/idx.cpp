//Copyright © 2022 Charles Kerr. All rights reserved.

#include "idx.hpp"

#include <iostream>

using namespace std::string_literals;
namespace ultima{
    //=================================================================================
    auto  idx_t::load(std::ifstream &input) ->void{
        input.read(reinterpret_cast<char*>(&offset),sizeof(offset)) ;
        input.read(reinterpret_cast<char*>(&length),sizeof(length)) ;
        input.read(reinterpret_cast<char*>(&extra),sizeof(extra)) ;
    }
    //=================================================================================
    auto idx_t::save(std::ofstream &output) ->void{
        output.write(reinterpret_cast<char*>(&offset),sizeof(offset)) ;
        output.write(reinterpret_cast<char*>(&length),sizeof(length)) ;
        output.write(reinterpret_cast<char*>(&extra),sizeof(extra)) ;
    }
    
    //=================================================================================
    auto gatherIDXOffsets(std::ifstream &input) ->std::vector<size_t> {
        input.seekg(0,std::ios::end);
        auto size = input.tellg() ;
        auto count = size/12 ;
        auto rvalue = std::vector<size_t>(count,0);
        for (size_t i = 0 ; i < count ; ++i){
            rvalue.at(i) = i*12 ;
        }
        return rvalue ;
    }
    //=================================================================================
    auto gatherIDXEntries( std::ifstream &input) ->std::map<std::uint32_t,idx_t> {
        auto rvalue = std::map<std::uint32_t,idx_t>() ;
        if (input.good() ){
            input.seekg(0,std::ios::beg);
            auto id = std::uint32_t(0);
            while (input.good() && !input.eof()) {
                auto idx = idx_t(input) ;
                if (idx.valid()){
                    rvalue.insert_or_assign(id,idx);
                }
                id++;
            }
        }
        return rvalue ;
    }
    //=================================================================================
    auto createIDX(std::ofstream &output, std::uint32_t numentries,std::uint32_t def_invalid_offset )->std::vector<std::uint64_t> {
        auto rvalue = std::vector<std::uint64_t>() ;
        if (output.good() ){
            output.seekp(0,std::ios::beg);
            auto idx = idx_t(def_invalid_offset);
            for (std::uint32_t j= 0 ; j< numentries ; j++){
                rvalue.push_back(static_cast<std::uint64_t>(output.tellp()));
                idx.save(output) ;
            }
        }
        return rvalue ;
        
    }
    
    //=================================================================================
    auto readMulData(std::ifstream &input,idx_t entry) ->std::vector<std::uint8_t> {
        auto buffer = std::vector<std::uint8_t>(entry.length,0);
        input.seekg(entry.offset,std::ios::beg);
        input.read(reinterpret_cast<char*>(buffer.data()),buffer.size());
        return buffer ;
    }
}
