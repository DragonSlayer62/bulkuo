//Copyright © 2022 Charles Kerr. All rights reserved.

#include "uowave.hpp"

#include <iostream>
#include <algorithm>
#include <stdexcept>
using namespace std::string_literals;
namespace ultima{
    //=================================================================================
    // uowave_t::header_t
    //=================================================================================
    //=================================================================================
    uowave_t::header_t::header_t():filesize(0),signature("RIFF"){
        
    }
    //=================================================================================
    auto uowave_t::header_t::load(std::ifstream &input) ->void {
        auto temp = std::uint32_t(0) ;
        input.read(reinterpret_cast<char*>(&temp),4);
        signature=std::string(reinterpret_cast<char*>(&temp),reinterpret_cast<char*>(&temp)+4);
        if (signature!= "RIFF"){
            throw std::runtime_error("Input is not a wav file");
        }
        input.read(reinterpret_cast<char*>(&filesize),4) ;
        
    }
    //=================================================================================
    auto uowave_t::header_t::save(std::ofstream &output) ->void {
        output.write(signature.c_str(),4);
        output.write(reinterpret_cast<char*>(&filesize),4);
    }
    //=================================================================================
    // uowave_t::wavfmt_t
    //=================================================================================
    
    //=================================================================================
    uowave_t::wavfmt_t::wavfmt_t() :type("WAVE"),signature("fmt "),size(16),format(1),channels(1),sample_rate(22050),byte_rate(44100),block_alignment(2),bits_per_sample(16){
        
    }
    //=================================================================================
    auto uowave_t::wavfmt_t::load(std::ifstream &input) ->void {
        auto temp = std::uint32_t(0) ;
        input.read(reinterpret_cast<char*>(&temp),4);
        type=std::string(reinterpret_cast<char*>(&temp),reinterpret_cast<char*>(&temp)+4);
        if (type!= "WAVE"){
            throw std::runtime_error("Input is not a wav file");
        }
        input.read(reinterpret_cast<char*>(&temp),4);
        signature=std::string(reinterpret_cast<char*>(&temp),reinterpret_cast<char*>(&temp)+4);
        if (signature!= "fmt "){
            throw std::runtime_error("Input is not a valid wav file (fmt header invalid");
        }
        input.read(reinterpret_cast<char*>(&size),4);
        input.read(reinterpret_cast<char*>(&format),2);
        if (format != 1){
            throw std::runtime_error("Input is not PCM format");
        }
        input.read(reinterpret_cast<char*>(&channels),2);
        if (channels != 1){
            throw std::runtime_error("Input must be single channel (mono)");
        }
        input.read(reinterpret_cast<char*>(&sample_rate),4);
        if (sample_rate != 22050){
            throw std::runtime_error("Input must be 22050 khz");
        }
        input.read(reinterpret_cast<char*>(&byte_rate),4);
        
        if (byte_rate != 44100){
            throw std::runtime_error("Byte rate must be 44100");
        }
        
        input.read(reinterpret_cast<char*>(&block_alignment),2);
        if (block_alignment != 2){
            throw std::runtime_error("Block aligment must be 2");
        }
        input.read(reinterpret_cast<char*>(&bits_per_sample),2);
        if (bits_per_sample != 16){
            throw std::runtime_error("Bits per sample must be 16");
        }
        if (size > 16) {
            input.seekg(size - 16, std::ios::cur);
        }
    }
    //=================================================================================
    auto uowave_t::wavfmt_t::save(std::ofstream &output) ->void  {
        output.write(type.c_str(),4);
        output.write(signature.c_str(),4);
        output.write(reinterpret_cast<char*>(&size),4);
        output.write(reinterpret_cast<char*>(&format),2);
        output.write(reinterpret_cast<char*>(&channels),2);
        output.write(reinterpret_cast<char*>(&sample_rate),4);
        output.write(reinterpret_cast<char*>(&byte_rate),4);
        output.write(reinterpret_cast<char*>(&block_alignment),2);
        output.write(reinterpret_cast<char*>(&bits_per_sample),2);
    }
    //=================================================================================
    auto uowave_t::wavfmt_t::chunksize() const ->std::int32_t {
        return static_cast<std::int32_t>(this->size + 8) ;
    }
    
    //=================================================================================
    // uowave_t::data-t
    //=================================================================================
    
    //=================================================================================
    uowave_t::data_t::data_t() :signature("data"),size(0){
        
    }
    //=================================================================================
    auto uowave_t::data_t::load(std::ifstream &input) ->void {
        auto temp = std::uint32_t(0) ;
        input.read(reinterpret_cast<char*>(&temp),4);
        signature=std::string(reinterpret_cast<char*>(&temp),reinterpret_cast<char*>(&temp)+4);
        if (signature!= "data"){
            throw std::runtime_error("Input data section is not valid (data header invalid");
        }
        input.read(reinterpret_cast<char*>(&size),4);
        audio = std::vector<std::uint8_t>((size - 8),0) ;
        input.read(reinterpret_cast<char*>(audio.data()),audio.size());
        if (input.gcount()!= audio.size()){
            throw std::runtime_error("Error reading audio data.");
        }
        
    }
    //=================================================================================
    auto uowave_t::data_t::save(std::ofstream &output) ->void {
        output.write(signature.c_str(),4);
        output.write(reinterpret_cast<char*>(&size),4);
        output.write(reinterpret_cast<char*>(audio.data()),audio.size());
        if (audio.size()%2 == 1){
            auto pad = char(0);
            output.write(&pad,1);
        }
    }
    //=================================================================================
    auto uowave_t::data_t::chunksize() const ->std::int32_t{
        return static_cast<std::int32_t>((audio.size()%2) + audio.size() + 8 );
    }
    //=================================================================================
    auto uowave_t::data_t::loadUO(const std::uint8_t *data , size_t amount) -> std::string{
        auto buffer = std::vector<std::uint8_t>(17,0) ;
        std::copy(data,data+16,buffer.data());
        std::string name = reinterpret_cast<char*>(buffer.data());
        buffer = std::vector<std::uint8_t>(amount-32,0) ;
        std::copy(data+32,data+amount,buffer.data());
        std::swap(audio,buffer) ;
        size = static_cast<std::int32_t>(audio.size());
        return name ;
    }
    //=================================================================================
    auto uowave_t::data_t::createUO(const std::string &name) const ->std::vector<std::uint8_t> {
        auto buffer = std::vector<std::uint8_t>(32+audio.size(),0) ;
        auto amount = std::min(static_cast<size_t>(16),name.size());
        std::copy(name.c_str(),name.c_str()+amount,reinterpret_cast<char*>(buffer.data()));
        std::copy(audio.begin(),audio.end(),buffer.begin()+32);
        return buffer ;
    }
    //=================================================================================
    // uowave_t
    //=================================================================================
    
    //=================================================================================
    auto uowave_t::load(const std::filesystem::path &wavpath) ->bool {
        auto input = std::ifstream(wavpath.string(),std::ios::binary);
        if (!input.is_open()){
            return false ;
        }
        header.load(input);
        format.load(input);
        data.load(input);
        return true ;
    }
    //=================================================================================
    auto uowave_t::save(const std::filesystem::path &wavpath)  ->bool {
        header.filesize = data.chunksize() + format.chunksize();
        auto output = std::ofstream(wavpath.string(),std::ios::binary);
        if (!output.is_open()){
            return false ;
        }
        header.save(output);
        format.save(output);
        data.save(output) ;
        return true ;
    }
    //=================================================================================
    auto uowave_t::save(std::ofstream &output)->void {
        header.filesize = data.chunksize() + format.chunksize();
        header.save(output);
        format.save(output);
        data.save(output) ;
    }

    //=================================================================================
    auto uowave_t::loadUO(const std::uint8_t * data, size_t length)->std::string {
        return this->data.loadUO(data, length);
    }
    //=================================================================================
    auto uowave_t::createUO(const std::string &name) const ->std::vector<std::uint8_t>{
        return this->data.createUO(name);
    }
    
    //=================================================================================
    // This is the first 16 bytes of the data, and it assumes the buffer is 17 bytes (last byte 0)
    auto nameForSound(const std::vector<char> &buffer) ->std::string {
        std::string name = buffer.data();
        if (!name.empty()) {
            auto path = std::filesystem::path(name);
            name = path.stem();
        }
        return name ;
    }
}
