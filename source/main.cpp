//Copyright © 2022 Charles Kerr. All rights reserved.

#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "argument.hpp"
#include "action/actions.hpp"


using namespace std::string_literals;
auto printHelp() ->void ;


int main(int argc, const char * argv[]) {
    const auto bulkuo_version ="Taffy"s;
    auto exitcode = EXIT_SUCCESS ;
    try {
        auto arg = argument_t(argc, argv);
        if (arg.help){
            printHelp() ;
        }
        else if (arg.version){
            std::cout <<"bulkuo version: "<<bulkuo_version<<std::endl;
        }
        else {
            auto iter = action_mapping.find(arg.action);
            if (iter != action_mapping.end()){
                auto siter = iter->second.find(arg.type);
                if (siter != iter->second.end()){
                    siter->second(arg,arg.type);
                }
            }
            
        }
    }
    catch(const std::exception &e){
        std::cerr <<e.what()<<std::endl;
        exitcode = EXIT_FAILURE ;
    }
    
    return exitcode;
}

//==================================================================================
auto printHelp() ->void {
    
}
