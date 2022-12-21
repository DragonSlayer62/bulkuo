//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef types_hpp
#define types_hpp

#include <cstdint>
#include <string>

//=================================================================================
//=================================================================================
enum class action_t {
    unknown,name,exist,create,merge,extract
};
auto nameForAction(action_t action) -> const std::string& ;
auto actionForName(const std::string &name) ->action_t ;


enum class datatype_t {
    unknown,art,texture,sound,gump,hue,multi,animation,info
};
auto nameForDatatype(datatype_t type) -> const std::string& ;
auto datatypeForName(const std::string &name) ->datatype_t ;


#endif /* types_hpp */
