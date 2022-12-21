//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef uowave_hpp
#define uowave_hpp

#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
namespace ultima{
    //=================================================================================
    class uowave_t {
        struct header_t {
            std::string signature ; // Must be RIFF
            std::int32_t filesize ; // Size of overall file, minus 8(the signature and file size holders)
            header_t() ;
            auto load(std::ifstream &input) ->void ;
            auto save(std::ofstream &output) ->void ;
        };
        struct wavfmt_t {
            std::string type ; //must be WAVE
            std::string signature ; // Must be "fmt " ;
            std::int32_t size ; // size of the structure, including sig/size (16)
            std::int16_t format ; // PCM = 1 ;
            std::int16_t channels ; // 1 for uo data
            std::int32_t sample_rate ; // 22050 for uo data
            std::int32_t byte_rate ; // (Sample Rate * BitsPerSample * Channels) / 8.
            std::int16_t block_alignment ;
            std::int16_t bits_per_sample ; // 16 for uo data (44100) for uo data
            wavfmt_t() ;
            auto load(std::ifstream &input) ->void ;
            auto save(std::ofstream &output) ->void ;
            auto chunksize() const ->std::int32_t ;
            
        };
        struct data_t {
            std::string signature ; // "data"
            std::int32_t size; // size of the data section
            std::vector<std::uint8_t> audio ;
            data_t() ;
            auto load(std::ifstream &input) ->void ;
            auto save(std::ofstream &output) ->void ;
            auto chunksize() const ->int32_t ;
            auto loadUO(const std::uint8_t *data , size_t amount)  ->std::string ;
            auto createUO(const std::string &name) const ->std::vector<std::uint8_t> ;
        };
        
        header_t header ;
        wavfmt_t format ;
        data_t data ;
        
    public:
        uowave_t() =default;
        auto load(const std::filesystem::path &wavpath) ->bool ;
        auto save(const std::filesystem::path &wavpath)  ->bool ;
        auto save(std::ofstream &output)->void ;
        auto loadUO(const std::uint8_t * data, size_t length)->std::string ;
        auto createUO(const std::string &name) const ->std::vector<std::uint8_t> ;
    };
    
    // This is the first 16 bytes of the data, and it assumes the buffer is 17 bytes (last byte 0)
    auto nameForSound(const std::vector<char> &buffer) ->std::string ;
}
#endif /* uowave_hpp */
