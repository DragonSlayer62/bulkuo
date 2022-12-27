//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef actutility_hpp
#define actutility_hpp

#include <cstdint>
#include <string>
#include <filesystem>
#include <fstream>

#include <map>
#include <set>

#include "../uodata/uop.hpp"
#include "../uodata/idx.hpp"
#include "../uodata/tileinfo.hpp"

#include "../types.hpp"
#include "../argument.hpp"
//=================================================================================
auto primaryForType(datatype_t type) ->std::string ;
auto secondaryForType(datatype_t type) ->std::string ;

auto idFromString(const std::string &value) ->std::uint32_t ;

auto contentsFor(const std::filesystem::path &path,const std::string &extension) ->std::map<std::uint32_t,std::filesystem::path> ;
auto validInContents(const argument_t &arg, const std::map<std::uint32_t,std::filesystem::path> &data)->std::set<std::uint32_t>;
auto nameInFile(const std::filesystem::path &path) ->std::string ;
auto unionOfId(const std::set<std::uint32_t> &ids, const std::map<std::uint32_t,ultima::table_entry> &mapping) ->std::set<std::uint32_t> ;
auto minIDXForType(datatype_t type) ->std::uint32_t ;
auto createUOPEntry(datatype_t type, std::uint32_t id,std::filesystem::path &path, std::ifstream &input,std::vector<std::uint8_t> &buffer) ->bool ;
auto createIDXEntry(datatype_t type, std::uint32_t id,std::filesystem::path &path, std::ifstream &input,std::vector<std::uint8_t> &buffer, ultima::idx_t &entry) ->void;
auto updateInfo(const argument_t &arg, std::ifstream &input, ultima::tileinfo_t &info)->std::uint32_t;
#endif /* actutility_hpp */
