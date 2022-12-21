//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef uop_hpp
#define uop_hpp

#include <cstdint>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <utility>
#include "hash.hpp"

namespace ultima {
    //================================================================================
    // A collection of functions to access and create uop files
    //=================================================================================
    // UOP files are used by Ultima Online for different data types.
    // The basic structure is a series of tables, that have entries for each data item
    // that says where in the file that data is, is it compressed, the length, etc.
    //
    // To make/read uops, the key is understanding where each table entry is for each
    // data item.  These utilities, help in facilating that activity
    //
    //=============================================================================
    // Hash formats for UO data
    // Gumps
    //	"build/gumpartlegacymul/%08u.tga"
    // Art
    //	"build/artlegacymul/%08u.tga"
    // Maps
    //	"build/map%ulegacymul/%08u.dat"  <<<<<<< the %u is the map number number
    // Sound
    //	"build/soundlegacymul/%08u.dat"
    // Multi
    //	"build/multicollection/%06u.bin"	Note:	this also contains a housing.bin
    //												file, that hash is 0x126D1E99DDEDEE0A
    //												(hashformat:build/multicollection/housing.bin)
    //
    //=================================================================================
    // UOP files are based a table of entries, that have identifers (hashes, hashLittle2 format)
    // size of the data, and if compressed or not.  These tables contain offsets to where the
    // the actual data for any given entry is.
    //=================================================================================
    struct table_entry {
        static constexpr auto entry_size = 34 ;
        std::uint64_t	offset ;				// Offset into the file where the data associated with the entry is located at
        std::uint32_t	header_length ;			// If that data has a header, the length of it. To offset to the data, you would do: offset + header_length ;
        std::uint32_t	compressed_length ;		// THe size of the compressed data (you use this to read the data)
        std::uint32_t	decompressed_length ;	// The size of the data after decompressing
        std::uint64_t	identifier ;			// What is the hash value for this entry
        std::uint32_t	data_block_hash ;		// The hash of the actual data (not sure how this is used)
        std::int16_t	compression ;			// If 1, then the data is compressed with zlib
        table_entry() ;
        table_entry(std::istream &input);
        auto load(std::istream &input) ->table_entry & ;
        auto valid() const ->bool;
        auto save(std::ostream &output) ->table_entry & ;
        auto description() const ->void ;
    };
    
    
    //=================================================================================
    // Just a check to ensure the uop has a signature and valid version
    auto validUOP(std::istream &input) ->bool ;
    
    //=================================================================================
    // This returns a the offsets for each table entry ;
    auto gatherEntryOffsets(std::istream &input) ->std::vector<std::uint64_t> ;
    //==================================================================================
    // This writes the uop header and initializes table entrys for the number of of items
    // Returns an array of offsets for each table_entry
    auto createUOP(std::ostream &output, std::uint32_t numitems) ->std::vector<std::uint64_t> ;
    
    
    //==========================================================================================
    // This taks a hashset_t (a series of hashes mapped to item ids), and returns a map of your item ids,
    // and their correponding table entry when reading an existing uop file. If they id is not present,
    // it will not be included in the map
    auto createIDTableMapping(std::istream &input, const hashset_t &hashmapping, const std::vector<std::uint64_t> &offsets ) ->std::map<std::uint32_t,table_entry> ;
    
    //===========================================================================================
    // Update the hashes
    auto updateBlockHash(std::iostream &stream) ->void ;
    
    //===========================================================================================
    auto readUOPData(const table_entry &entry, std::ifstream &input) ->std::vector<std::uint8_t> ;
    auto compressUOPData(const std::vector<std::uint8_t> &data) ->std::vector<std::uint8_t> ;
}
#endif /* uop_hpp */
