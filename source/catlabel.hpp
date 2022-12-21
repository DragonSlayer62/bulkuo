//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef catlabel_hpp
#define catlabel_hpp

#include <cstdint>
#include <string>
#include <filesystem>
#include <map>
//=================================================================================
class fileparser_t {
  
protected:
    virtual auto lineentry(const std::string &left,const std::string &right) ->void{} 
    auto parse(const std::filesystem::path &path) ->void ;
public:
    fileparser_t() = default ;
 };

//=================================================================================
class label_t : public fileparser_t{
    std::map<std::uint32_t,std::string> labels ;
    static const std::string empty_string ;
protected:
    auto lineentry(const std::string &left, const std::string &right) ->void final;
public:
    auto label(std::uint32_t id) const ->const std::string& ;
    label_t(const std::filesystem::path &path);
    label_t() = default;
};

//=================================================================================
class category_t : public fileparser_t {
    std::map<std::uint32_t, std::filesystem::path> categories ;
protected:
    auto lineentry(const std::string &left, const std::string &right) ->void final;
public:
    auto category(std::uint32_t id,const std::filesystem::path &directory) const ->std::filesystem::path ;
    category_t(const std::filesystem::path &path);
    category_t()=default ;

};
#endif /* catlabel_hpp */
