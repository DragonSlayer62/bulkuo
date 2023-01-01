//Copyright © 2022 Charles Kerr. All rights reserved.

#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "argument.hpp"
#include "action/actions.hpp"


using namespace std::string_literals;
auto printHelp() ->void ;

//==============================================================================================
int main(int argc, const char * argv[]) {
    const auto bulkuo_version ="Taffy"s;
    auto exitcode = EXIT_SUCCESS ;
    try {
        auto arg = argument_t(argc, argv);
        if (arg.help || arg.type == datatype_t::unknown || arg.action == action_t::unknown){
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
    std::cout <<std::endl;
    std::cout <<"Usage:\n";
    std::cout <<"    bulkuo [flag,...] [action] [data] path path ...\n";
    std::cout <<"Where:\n";
    std::cout <<"    Flags (optional):\n";
    std::cout <<"        --bmp24             (24 bit bmp files will be created, versus 16 bit (native UO color size)\n";
    std::cout <<"        --category=filename (Puts the file in a subdirectory specified in the file based on id)\n";
    std::cout <<"        --help              (Prints usage information, all other parametes are ignored)\n";
    std::cout <<"        --hex               (Tileids will outputed in hex)\n";
    std::cout <<"        --id=f:filename\n";
    std::cout <<"             or\n";
    std::cout <<"        --id=l:id list      (Restricts the action to only the ids constrained by the flag value)\n";
    std::cout <<"        --label=filename    (Appends a label=, based on id, to the filename created on extraction)\n";
    std::cout <<"        --overwrite         (Allow overwriting files that exist)\n";
    std::cout <<"        --version           (Prints the version, all other parameters are ignored)\n";
    std::cout <<"    Action (one of the following)\n";
    std::cout <<"        --create            (Creates the requested file format for the specified data type)\n";
    std::cout <<"        --exist             (Prints to std output the ids that have information in the data)\n";
    std::cout <<"        --extract           (Extracts the data from uo format for the data specified)\n";
    std::cout <<"        --name              (For 'info,multi,sound,hue' will generate names for an id if one is present)\n";
    std::cout <<"        --merge             (Will merge the data into an existing uo format)\n";
    std::cout <<"    Data (one of the following)\n";
    std::cout <<"        --animation         (animation  data, bmp files and csv files)\n";
    std::cout <<"        --art               (art  data, bmp files)\n";
    std::cout <<"        --gump              (gump  data, bmp files)\n";
    std::cout <<"        --hue               (hue data, bmp and txt files)\n";
    std::cout <<"        --info              (tile information  data, csv file)\n";
    std::cout <<"        --light             (ligh  data, bmp files)\n";
    std::cout <<"        --multi             (multi  data, csv files)\n";
    std::cout <<"        --sound             (sound data, wav and txt files)\n";
    std::cout <<"        --texture           (texture  data, bmp files)\n";
    std::cout <<"    Paths\n";
    std::cout <<"        exist\n";
    std::cout <<"               idx_path mul_path\n";
    std::cout <<"               uop_path \n";
    std::cout <<"               huemul_path\n";
    std::cout <<"        extract\n";
    std::cout <<"               idx_path mul_path out_directory\n";
    std::cout <<"               uop_path out_directory\n";
    std::cout <<"               huemul_path out_directory\n";
    std::cout <<"               mul_path csv_file out_\n";
    std::cout <<"               anim_idx_path anim_mul_path image_csv_directory [swap_directory]\n";
    std::cout <<"        create\n";
    std::cout <<"               directory_path out_idx_path out_mul_path\n";
    std::cout <<"               directory_path out_uop_path \n";
    std::cout <<"               directory out_huemul_path \n";
    std::cout <<"               info_csv_file out_mul_path \n";
    std::cout <<"               image_csv_directory [swap_directory] anim_idx_path anim_mul_path\n";
    std::cout <<"        merge (creates \".bulkuo\" versions of the uo format)\n";
    std::cout <<"               directory_path in_idx_path in_mul_path\n";
    std::cout <<"               directory_path in_uop_path \n";
    std::cout <<"               directory in_huemul_path \n";
    std::cout <<"               csv_file in_mul_path \n";
    std::cout <<"               image_csv_directory [swap_directory] anim_idx_path anim_mul_path\n";
    std::cout <<"        name\n";
    std::cout <<"               hue_mul_path out_name_file\n";
    std::cout <<"               sound_idx_path sound_mul_path out_name_file\n";
    std::cout <<"               sound_uop_path out_name_file\n";
    std::cout <<"               out_multi_name_file\n";
    std::cout <<"               info_mul_path out_name_file\n";
    std::cout <<std::endl;
}
