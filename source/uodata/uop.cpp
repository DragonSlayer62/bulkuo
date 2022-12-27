//Copyright © 2022 Charles Kerr. All rights reserved.

#include "uop.hpp"

#include <iostream>
#include <zlib.h>

using namespace std::string_literals;

namespace ultima{
    //================================================================================
    // Define the uop signature and the version we support
    constexpr 	auto uop_identifer = std::uint32_t(0x50594D);
    constexpr	auto uop_version = std::uint32_t(5) ;
    
    // Define where the start table offset is define
    constexpr	auto table_offset_location = std::uint32_t(12) ;
    
    // Define where the tables start for writing purposes
    constexpr	auto starting_offset = std::uint64_t(512) ;
    
    
    
    // Define the default table_size
    constexpr   auto default_table_size  = std::uint32_t(1000) ;
    
    //=================================================================================
    //=================================================================================
    table_entry::table_entry():offset(0),header_length(0),compressed_length(0),decompressed_length(0),identifier(0),data_block_hash(0),compression(0) {
    }
    //=================================================================================
    table_entry::table_entry(std::istream &input):table_entry() {
        load(input) ;
    }
    //=================================================================================
    auto table_entry::load(std::istream &input) ->table_entry & {
        input.read(reinterpret_cast<char*>(&offset),sizeof(offset));
        input.read(reinterpret_cast<char*>(&header_length),sizeof(header_length));
        input.read(reinterpret_cast<char*>(&compressed_length),sizeof(compressed_length));
        input.read(reinterpret_cast<char*>(&decompressed_length),sizeof(decompressed_length));
        input.read(reinterpret_cast<char*>(&identifier),sizeof(identifier));
        input.read(reinterpret_cast<char*>(&data_block_hash),sizeof(data_block_hash));
        input.read(reinterpret_cast<char*>(&compression),sizeof(compression));
        return *this ;
    }
    //=================================================================================
    auto table_entry::save(std::ostream &output) ->table_entry & {
        output.write(reinterpret_cast<char*>(&offset),sizeof(offset));
        output.write(reinterpret_cast<char*>(&header_length),sizeof(header_length));
        output.write(reinterpret_cast<char*>(&compressed_length),sizeof(compressed_length));
        output.write(reinterpret_cast<char*>(&decompressed_length),sizeof(decompressed_length));
        output.write(reinterpret_cast<char*>(&identifier),sizeof(identifier));
        output.write(reinterpret_cast<char*>(&data_block_hash),sizeof(data_block_hash));
        output.write(reinterpret_cast<char*>(&compression),sizeof(compression));
        return *this ;
    }
    //=================================================================================
    auto table_entry::valid() const ->bool {
        return (identifier != 0) && (decompressed_length!=0) ;
    }
    //=================================================================================
    auto table_entry::description() const ->void {
        // The hash
        std::cout <<"hash: "<<identifier<<std::endl;
        std::cout <<"offset: "<< offset << std::endl;
        std::cout <<"header length: " << header_length<<std::endl;
        std::cout <<"compressed length: " << compressed_length<<std::endl;
        std::cout <<"decompressed length: " << decompressed_length<<std::endl;
        std::cout <<"compressed: " << compression<<std::endl;
        std::cout <<"data block hash: "<<data_block_hash<<std::endl;
    }
    //========================================================================================
    auto validUOP(std::istream &input) ->bool {
        auto rvalue = false ;
        if (input.good()) {
            auto current = input.tellg() ;
            input.seekg(0,std::ios::beg) ;
            auto value = std::uint32_t(0);
            input.read(reinterpret_cast<char*>(&value),4);
            if (value == uop_identifer) {
                input.read(reinterpret_cast<char*>(&value),4);
                if (value <= uop_version) {
                    rvalue = true ;
                }
            }
            input.seekg(current,std::ios::beg);
        }
        return rvalue ;
    }
    //=========================================================================================
    auto gatherEntryOffsets(std::istream &input) ->std::vector<std::uint64_t>{
        auto rvalue = std::vector<std::uint64_t>() ;
        if (input.good()){
            auto current = input.tellg() ;
            input.seekg(table_offset_location,std::ios::beg);
            
            auto location = std::uint64_t(0) ;
            
            input.read(reinterpret_cast<char*>(&location),sizeof(location));
            
            // There is some other useful information, like tablesize, and number of entries
            // but we dont really need it to gather the table_entries ;
            auto tablesize = std::uint32_t(0);
            do {
                input.seekg(location,std::ios::beg) ;
                input.read(reinterpret_cast<char*>(&tablesize),sizeof(tablesize));
                input.read(reinterpret_cast<char*>(&location),sizeof(location));
                auto entryoffset = static_cast<std::uint64_t>(input.tellg());
                for (std::uint32_t j=0 ; j < tablesize;j++){
                    rvalue.push_back(entryoffset + static_cast<std::uint64_t>(j*table_entry::entry_size));
                }
                
            }while((location != 0) && (input.good()));
            
            input.seekg(current,std::ios::beg);
        }
        return rvalue ;
    }
    
    //=========================================================================================
    auto createUOP(std::ostream &output, std::uint32_t numentries) ->std::vector<std::uint64_t> {
        auto rvalue = std::vector<std::uint64_t>() ;
        if (output.good()){
            output.write(reinterpret_cast<const char*>(&uop_identifer),sizeof(uop_identifer));
            output.write(reinterpret_cast<const char*>(&uop_version),sizeof(uop_version));
            auto value = std::uint32_t(0xFD23EC43);// No idea if this needs to be this value, or 0, or whatever
            output.write(reinterpret_cast<char*>(&value),sizeof(value));
            // Were does the first table reside in the file?
            output.write(reinterpret_cast<const char*>(&starting_offset),sizeof(starting_offset));
            // table size
            output.write(reinterpret_cast<const char*>(&default_table_size),sizeof(default_table_size));
            // Number of entries
            output.write(reinterpret_cast<const char*>(&numentries),sizeof(numentries));
            // No idea if the next values need to be 1, 0 , or doesnt matter, so will copy what one was
            value = 1 ;
            output.write(reinterpret_cast<char*>(&value),sizeof(value));
            output.write(reinterpret_cast<char*>(&value),sizeof(value));
            value = 0 ;
            output.write(reinterpret_cast<char*>(&value),sizeof(value));
            
            // Ok, now we will fill in the rest of space until the table offset with zeros
            auto byte = std::uint8_t(0) ;
            auto temploc = static_cast<std::uint32_t>(output.tellp());
            for (std::uint32_t j= temploc ; j < starting_offset ; j++){
                output.write(reinterpret_cast<char*>(&byte),1);
            }
            // Ok, this is is the start of the tables
            // how many tables?
            auto numtables = numentries/default_table_size + (numentries%default_table_size>0?1:0) ;
            auto entry = table_entry() ;
            auto entryoffset = std::uint64_t(0) ;
            for (std::uint32_t j=0 ; j< numtables;j++){
                // We need to write the table size
                auto tablesize = default_table_size ;
                auto nextlocation = static_cast<std::uint64_t>(output.tellp()) + static_cast<std::uint64_t>(table_entry::entry_size * tablesize) + 12 ;// 12 is the tablesize and nextlocation sizes
                if (numentries<default_table_size){
                    tablesize = numentries ;
                    nextlocation = 0 ;
                }
                output.write(reinterpret_cast<const char*>(&tablesize),sizeof(tablesize));
                output.write(reinterpret_cast<const char*>(&nextlocation),sizeof(nextlocation));
                numentries -= tablesize ;
                for (std::uint32_t i=0 ; i < tablesize;i++){
                    entryoffset = static_cast<std::uint64_t>(output.tellp()) ;
                    rvalue.push_back(entryoffset);
                    entry.save(output);
                }
            }
        }
        return rvalue ;
        
    }
    
    //=========================================================================================================================================
    auto createIDTableMapping(std::istream &input, const hashset_t &hashmapping, const std::vector<std::uint64_t> &offsets ) ->std::map<std::uint32_t,table_entry> {
        auto rvalue = std::map<std::uint32_t,table_entry>();
        if (input.good()){
            auto current = static_cast<std::uint64_t>(input.tellg());
            for (auto offset : offsets){
                input.seekg(offset,std::ios::beg);
                auto entry = table_entry(input) ;
                if (entry.valid()){
                    try {
                        auto id = hashmapping[entry.identifier] ;
                        rvalue.insert_or_assign(id, entry);
                    }
                    catch(...) {
                        // That hash wasnt in the mapping, so skip it
                    }
                }
                // Do we really need to keep reading if we found all the entries in the hashmapping?
                if (hashmapping.size() == rvalue.size()){
                    break;
                }
            }
            input.seekg(current,std::ios::beg);
        }
        return rvalue ;
    }
    
    //===========================================================================================
    // Update the hashes
    auto updateBlockHash(std::iostream &stream) ->void{
        if (validUOP(stream)){
            auto entries = gatherEntryOffsets(stream);
            for (auto offset:entries){
                stream.seekg(offset,std::ios::beg) ;
                
                auto entry = table_entry(stream) ;
                if (entry.valid()){
                    stream.seekg(entry.offset+entry.header_length,std::ios::beg);
                    entry.data_block_hash = hashAdler32(stream, entry.compressed_length) ;
                    stream.seekp(offset,std::ios::beg);
                    entry.save(stream) ;
                }
            }
        }
    }
    
    //===========================================================================================
    auto readUOPData(const table_entry &entry, std::ifstream &input) ->std::vector<std::uint8_t> {
        auto buffer = std::vector<std::uint8_t>(entry.compressed_length,0) ;
        input.seekg(entry.offset+entry.header_length,std::ios::beg);
        input.read(reinterpret_cast<char*>(buffer.data()),buffer.size());
        if (entry.compression){
            auto temp = std::vector<std::uint8_t>(entry.decompressed_length,0);
            uLong destlen = static_cast<uLongf>(temp.size()) ;
            uLong sourcelen = static_cast<uLongf>(buffer.size()) ;
            auto status = uncompress2(reinterpret_cast<Bytef *>(temp.data()), &destlen, reinterpret_cast<const Bytef *>(buffer.data()), &sourcelen);
            if (status != Z_OK){
                throw std::runtime_error("Error decompressing data.");
            }
            temp.resize(destlen);
            std::swap(buffer,temp);
        }
        return buffer ;
    }

    //===========================================================================================
    auto compressUOPData(const std::vector<std::uint8_t> &data) ->std::vector<std::uint8_t> {
        auto sourcesize = static_cast<uLong>(data.size());
        auto roughsize = compressBound(sourcesize);
        auto dest = std::vector<std::uint8_t>(roughsize,0) ;
        auto status = compress(dest.data(), &roughsize, data.data(), sourcesize) ;
        if (status != Z_OK) {
            throw std::runtime_error("Error compressing data."s );
        }
        dest.resize(roughsize);
        return dest ;
     }

}
