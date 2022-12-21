//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef idlist_hpp
#define idlist_hpp

#include <cstdint>
#include <string>
#include <set>
#include <filesystem>
//=================================================================================
auto setToList(const std::set<std::uint32_t> &values,bool use_hex) ->std::string;

auto processList(const std::string &line) ->std::set<std::uint32_t> ;

auto processList(const std::filesystem::path &path) ->std::set<std::uint32_t> ;
#endif /* idlist_hpp */
