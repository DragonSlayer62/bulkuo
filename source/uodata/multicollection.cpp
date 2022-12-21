//

#include "multicollection.hpp"

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <algorithm>
#include <sstream>
#include "strutil.hpp"
using namespace std::string_literals;
namespace ultima {
    //=================================================================================
    // multi_sub_item_t
    //=================================================================================
    
    //=================================================================================
    const std::string multi_sub_item_t::header_line="tileid,x_offset,y_offset,z_offset,flag,cliloc";

    //=================================================================================
    multi_sub_item_t::multi_sub_item_t():tileid(0),xoffset(0),yoffset(0),zoffset(0),flag(0){
        
    }
    
    //=================================================================================
    multi_sub_item_t::multi_sub_item_t(const std::string &line):multi_sub_item_t(){
        auto values = strutil::parse(line,",");
        switch (values.size()){
            default:
            case 6: {
                auto clis = strutil::parse(values.at(5),":");
                for (size_t j=0 ;j<clis.size();j++){
                    cliloc.push_back(static_cast<std::uint32_t>(std::stoul(clis.at(j),nullptr,0)));
                }
                [[fallthrough]];
            }
            case 5:
                this->flag =std::stoull(values.at(4),nullptr,0);
                [[fallthrough]];
            case 4:
                this->zoffset =static_cast<std::int16_t>(std::stoi(values.at(3),nullptr,0));
                [[fallthrough]];
            case 3:
                this->yoffset =static_cast<std::int16_t>(std::stoi(values.at(2),nullptr,0));
                [[fallthrough]];
            case 2:
                this->xoffset =static_cast<std::int16_t>(std::stoi(values.at(1),nullptr,0));
                [[fallthrough]];
            case 1:
                this->tileid =static_cast<std::uint16_t>(std::stoul(values.at(0),nullptr,0));
                [[fallthrough]];
            case 0:
                break;
        }

    }
    //=================================================================================
    auto multi_sub_item_t::description(bool use_hex) const ->std::string {
        std::stringstream output ;
        std::stringstream cli ;
        if (use_hex) {
            output <<std::hex<<std::showbase<<std::setw(4)<<tileid<<std::dec<<std::noshowbase<<",";
        }
        else {
            output <<tileid<<",";
        }
        output << xoffset<<","<<yoffset<<","<<zoffset<<"," ;
        output <<std::hex<<std::showbase<<std::setw(16)<<flag<<std::dec<<std::noshowbase<<",";
        for (const auto &value:cliloc) {
            if (!cli.str().empty()){
                cli<<":";
            }
            cli << value ;
        }
        output <<cli.str();
        return output.str() ;
    }
    //=================================================================================
    auto multi_sub_item_t::data(bool isuop) const ->std::vector<std::uint8_t>{
        auto buffer = std::vector<std::uint8_t>(8 + (isuop ? 6 + (4*cliloc.size()):8),0) ;
        std::copy(reinterpret_cast<const std::uint8_t*>(&tileid),reinterpret_cast<const std::uint8_t*>(&tileid)+2,buffer.data());
        std::copy(reinterpret_cast<const std::uint8_t*>(&xoffset),reinterpret_cast<const std::uint8_t*>(&xoffset)+2,buffer.data()+2);
        std::copy(reinterpret_cast<const std::uint8_t*>(&yoffset),reinterpret_cast<const std::uint8_t*>(&yoffset)+2,buffer.data()+4);
        std::copy(reinterpret_cast<const std::uint8_t*>(&zoffset),reinterpret_cast<const std::uint8_t*>(&zoffset)+2,buffer.data()+6);
        if (!isuop) {
            std::copy(reinterpret_cast<const std::uint8_t*>(&flag),reinterpret_cast<const std::uint8_t*>(&flag)+8,buffer.data()+8);
        }
        else {
            auto small_flag = std::uint16_t(0);
            if (flag ==0x0000000100000001) {
                small_flag = 256;
            }
            else if (flag ==  0x0000000100000000) {
                small_flag=257;
            }
            else if (flag ==0){
                small_flag = 1;
            }
            std::copy(reinterpret_cast<const std::uint8_t*>(&small_flag),reinterpret_cast<const std::uint8_t*>(&small_flag)+2,buffer.data()+8);
            auto count = static_cast<std::uint32_t>(cliloc.size());
            std::copy(reinterpret_cast<const std::uint8_t*>(&count),reinterpret_cast<const std::uint8_t*>(&count)+4,buffer.data()+10);
            for (size_t j=0 ; j< cliloc.size();j++){
                std::copy(reinterpret_cast<const std::uint8_t*>(&cliloc[j]),reinterpret_cast<const std::uint8_t*>(&cliloc[j])+4,buffer.data()+14+(j*4));
            }
        }
        return buffer ;
    }
    
    //=================================================================================
    // multi_entry_t
    //=================================================================================
    //=========================================================
    const std::map<std::uint32_t, std::string> multi_entry_t::names = {
        { 0, "Small Boat [north]"s }, { 1, "Small Boat [east]"s },
        { 2, "Small Boat [south]"s }, { 3, "Small Boat [west]"s },
        { 4, "Small Dragon Boat [north]"s }, { 5, "Small Dragon Boat [east]"s },
        { 6, "Small Dragon Boat [south]"s }, { 7, "Small Dragon Boat [west]"s },
        { 8, "Medium Boat [north]"s }, { 9, "Medium Boat [east]"s },
        { 10, "Medium Boat [south]"s }, { 11, "Medium Boat [west]"s },
        { 12, "Medium Dragon Boat [north]"s }, { 13, "Medium Dragon Boat [east]"s },
        { 14, "Medium Dragon Boat [south]"s }, { 15, "Medium Dragon Boat [west]"s },
        { 16, "Large Boat [north]"s }, { 17, "Large Boat [east]"s },
        { 18, "Large Boat [soutch]"s }, { 19, "Large Boat [west]"s },
        { 20, "Large Dragon Boat [north]"s }, { 21, "Large Dragon Boat [east]"s },
        { 22, "Large Dragon Boat [south]"s }, { 23, "Large Dragon Boat [west]"s },
        { 24, "Orcish Galleon [north]"s }, { 25, "Orcish Galleon [east]"s },
        { 26, "Orcish Galleon [south]"s }, { 27, "Orcish Galleon [west]"s },
        { 28, "Orcish Galleon [damaged][north]"s }, { 29, "Orcish Galleon [damaged][east]"s },
        { 30, "Orcish Galleon [damaged][south]"s }, { 31, "Orcish Galleon [damaged][west]"s },
        { 32, "Orcish Galleon [destroyed][north]"s }, { 33, "Orcish Galleon [destroyed][east]"s },
        { 34, "Orcish Galleon [destroyed][south]"s }, { 35, "Orcish Galleon [destroyed][west]"s },
        
        { 36, "Gargish Galleon [north]"s }, { 37, "Gargish Galleon [east]"s },
        { 38, "Gargish Galleon [south]"s }, { 39, "Gargish Galleon [west]"s },
        { 40, "Gargish Galleon [damaged][north]"s }, { 41, "Gargish Galleon [damaged][east]"s },
        { 42, "Gargish Galleon [damaged][south]"s }, { 43, "Gargish Galleon [damaged][west]"s },
        { 44, "Gargish Galleon [destroyed][north]"s }, { 45, "Gargish Galleon [destroyed][east]"s },
        { 46, "Gargish Galleon [destroyed][south]"s }, { 47, "Gargish Galleon [destroyed][west]"s },
        
        { 48, "Tokuno Galleon [north]"s }, { 49, "Tokuno Galleon [east]"s },
        { 50, "Tokuno Galleon [south]"s }, { 51, "Tokuno Galleon [west]"s },
        { 52, "Tokuno Galleon [damaged][north]"s }, { 53, "Tokuno Galleon [damaged][east]"s },
        { 54, "Tokuno Galleon [damaged][south]"s }, { 55, "Tokuno Galleon [damaged][west]"s },
        { 56, "Tokuno Galleon [destroyed][north]"s }, { 57, "Tokuno Galleon [destroyed][east]"s },
        { 58, "Tokuno Galleon [destroyed][south]"s }, { 59, "Tokuno Galleon [destroyed][west]"s },
        
        { 60, "Rowboat [north]"s }, { 61, "Rowboat [east]"s },
        { 62, "Rowboat [south]"s }, { 63, "Rowboat [west]"s },
        
        { 64, "British Galleon [north]"s }, { 65, "British Galleon [east]"s },
        { 66, "British Galleon [south]"s }, { 67, "British Galleon [west]"s },
        { 68, "British Galleon [damaged][north]"s }, { 69, "British Galleon [damaged][east]"s },
        { 70, "British Galleon [damaged][south]"s }, { 71, "British Galleon [damaged][west]"s },
        
        { 100, "Small Stone and Plaster House"s }, { 101, "Small Stone and Plaster House"s },
        { 102, "Small Fieldstone House"s }, { 103, "Small Fieldstone House"s },
        { 104, "Small Brick House"s }, { 105, "Small Brick House"s },
        { 106, "Small Wood House"s }, { 107, "Small Wood House"s },
        { 108, "Small Wood and Plaster House"s }, { 109, "Small Wood and Plaster House"s },
        { 110, "Small Thatched Roof Cottage"s }, { 111, "Small Thatched Roof Cottage"s },
        { 112, "Tent [blue]"s }, { 113, "Tent [blue]"s },
        { 114, "Tent [green]"s }, { 115, "Tent [green]"s },
        { 116, "Large Brick House"s }, { 117, "Large Brick House"s },
        { 118, "Two Story Wood and Plaster House"s }, { 119, "Two Story Wood and Plaster House"s },
        { 120, "Two Story Stone and Plaster House"s }, { 121, "Two Story Stone and Plaster House"s },
        { 122, "Large Tower"s }, { 123, "Large Tower"s },
        { 124, "Stone Keep"s }, { 125, "Stone Keep"s },
        { 126, "Castle"s }, { 127, "Castle"s },
        { 128, "Large Patio House"s }, { 129, "Large Patio House"s },
        { 141, "Large Patio House"s }, { 150, "Large Marble Patio House"s },
        { 152, "Small Tower"s }, { 154, "Log Cabin"s },
        { 156, "Sandstone Patio House"s }, { 158, "Two-Story Villa"s },
        { 160, "Small Stone Workshop"s }, { 162, "Small Marble Workshop"s },
        { 201, "Mine's Elevator"s }, { 202, "Lifting Bridge"s },
        { 500, "Wandering Healer Camp"s }, { 501, "Wandering Mage Camp"s },
        { 502, "Wandering Bank Camp"s },
        
        { 1000, "Treasure Pile"s }, { 1001, "Treasure Pile"s },
        { 1002, "Treasure Pile"s }, { 1003, "Treasure Pile"s },
        
        { 2000, "Wooden Supports"s }, { 2001, "Wooden Supports"s },
        { 2002, "Wooden Supports"s }, { 2003, "Wooden Supports"s },
        { 2004, "Wooden Supports"s }, { 2005, "Wooden Supports"s },
        { 2006, "Wooden Supports"s }, { 2007, "Wooden Supports"s },
        
        { 3000, "Farmer's Cabin"s },
        
        { 4000, "Exodus Generator"s }, { 4001, "Exodus Generator"s },
        { 4002, "Exodus Control Pylon"s },
        
        { 4003, "Nystul's Tower (stage 1)"s }, { 4004, "Nystul's Tower (stage 2)"s },
        { 4005, "Nystul's Tower (stage 3)"s }, { 4006, "Nystul's Tower (stage 4)"s },
        { 4007, "Nystul's Tower (stage 5)"s }, { 4008, "Nystul's Tower (stage 6)"s },
        
        { 4009, "Scaffold and Trailer"s }, { 4010, "Exodus Energy Tower"s },
        { 4011, "Exodus Barrier"s }, { 4012, "Exodus Control Device"s },
        { 5000, "Minax Stronghold"s },
        
        { 5100, "Foundation [7x7]"s }, { 5101, "Foundation [7x8]"s },
        { 5102, "Foundation [7x9]"s }, { 5103, "Foundation [7x10]"s },
        { 5104, "Foundation [7x11]"s }, { 5105, "Foundation [7x12]"s },
        
        { 5112, "Foundation [8x7]"s }, { 5113, "Foundation [8x8]"s },
        { 5114, "Foundation [8x9]"s }, { 5115, "Foundation [8x10]"s },
        { 5116, "Foundation [8x11]"s }, { 5117, "Foundation [8x12]"s },
        { 5118, "Foundation [8x13]"s },
        
        { 5124, "Foundation [9x7]"s }, { 5125, "Foundation [9x8]"s },
        { 5126, "Foundation [9x9]"s }, { 5127, "Foundation [9x10]"s },
        { 5128, "Foundation [9x11]"s }, { 5129, "Foundation [9x12]"s },
        { 5130, "Foundation [9x13]"s }, { 5131, "Foundation [9x14]"s },
        
        { 5136, "Foundation [10x7]"s }, { 5137, "Foundation [10x8]"s },
        { 5138, "Foundation [10x9]"s }, { 5139, "Foundation [10x10]"s },
        { 5140, "Foundation [10x11]"s }, { 5141, "Foundation [10x12]"s },
        { 5142, "Foundation [10x13]"s }, { 5143, "Foundation [10x14]"s },
        { 5144, "Foundation [10x15]"s },
        
        { 5148, "Foundation [11x7]"s }, { 5149, "Foundation [11x8]"s },
        { 5150, "Foundation [11x9]"s }, { 5151, "Foundation [11x10]"s },
        { 5152, "Foundation [11x11]"s }, { 5153, "Foundation [11x12]"s },
        { 5154, "Foundation [11x13]"s }, { 5155, "Foundation [11x14]"s },
        { 5156, "Foundation [11x15]"s }, { 5157, "Foundation [11x16]"s },
        
        
        { 5160, "Foundation [12x7]"s }, { 5161, "Foundation [12x8]"s },
        { 5162, "Foundation [12x9]"s }, { 5163, "Foundation [12x10]"s },
        { 5164, "Foundation [12x11]"s }, { 5165, "Foundation [12x12]"s },
        { 5166, "Foundation [12x13]"s }, { 5167, "Foundation [12x14]"s },
        { 5168, "Foundation [12x15]"s }, { 5169, "Foundation [12x16]"s },
        { 5170, "Foundation [12x17]"s },
        
        
        { 5173, "Foundation [13x8]"s }, { 5174, "Foundation [13x9]"s },
        { 5175, "Foundation [13x10]"s }, { 5176, "Foundation [13x11]"s },
        { 5177, "Foundation [13x12]"s }, { 5178, "Foundation [13x13]"s },
        { 5179, "Foundation [13x14]"s }, { 5180, "Foundation [13x15]"s },
        { 5181, "Foundation [13x16]"s }, { 5182, "Foundation [13x17]"s },
        { 5183, "Foundation [13x18]"s },
        
        { 5186, "Foundation [14x9]"s }, { 5187, "Foundation [14x10]"s },
        { 5188, "Foundation [14x11]"s }, { 5189, "Foundation [14x12]"s },
        { 5190, "Foundation [14x13]"s }, { 5191, "Foundation [14x14]"s },
        { 5192, "Foundation [14x15]"s }, { 5193, "Foundation [14x16]"s },
        { 5194, "Foundation [14x17]"s }, { 5195, "Foundation [14x18]"s },
        
        { 5199, "Foundation [15x10]"s }, { 5200, "Foundation [15x11]"s },
        { 5201, "Foundation [15x12]"s }, { 5202, "Foundation [15x13]"s },
        { 5203, "Foundation [15x14]"s }, { 5204, "Foundation [15x15]"s },
        { 5205, "Foundation [15x16]"s }, { 5206, "Foundation [15x17]"s },
        { 5207, "Foundation [15x18]"s },
        
        { 5212, "Foundation [16x11]"s }, { 5213, "Foundation [16x12]"s },
        { 5214, "Foundation [16x13]"s }, { 5215, "Foundation [16x14]"s },
        { 5216, "Foundation [16x15]"s }, { 5217, "Foundation [16x16]"s },
        { 5218, "Foundation [16x17]"s }, { 5219, "Foundation [16x18]"s },
        
        { 5225, "Foundation [17x12]"s }, { 5226, "Foundation [17x13]"s },
        { 5227, "Foundation [17x14]"s }, { 5228, "Foundation [17x15]"s },
        { 5229, "Foundation [17x16]"s }, { 5230, "Foundation [17x17]"s },
        { 5231, "Foundation [17x18]"s },
        
        { 5238, "Foundation [18x13]"s }, { 5239, "Foundation [18x14]"s },
        { 5240, "Foundation [18x15]"s }, { 5241, "Foundation [18x16]"s },
        { 5242, "Foundation [18x17]"s }, { 5243, "Foundation [18x18]"s },
        
        { 7500, "New Player Quest Camp"s }
    };


    //=================================================================================
    auto multi_entry_t::data(bool isuop) const ->std::vector<std::uint8_t> {
        auto buffer = std::vector<std::uint8_t>() ;
        for (const auto &entry:items){
            auto data = entry.data(isuop) ;
            buffer.insert(buffer.end(),data.begin(),data.end());
        }
        return buffer ;
    }
    
    //=================================================================================
    auto multi_entry_t::load(const std::vector<std::uint8_t> &data, bool isuop) ->void {
        if (data.size() < multi_sub_item_t::min_size) {
            throw std::runtime_error("Invalid multi entry data size.");
        }
        auto offset = size_t(0) ;
        auto count = std::uint32_t(0);
        auto small_flag = std::uint16_t(0);
        auto cli = std::uint32_t(0);
        items.clear();
        minz = std::numeric_limits<std::int16_t>::max();
        maxz = std::numeric_limits<std::int16_t>::min();
        miny = std::numeric_limits<std::int16_t>::max();
        maxy = std::numeric_limits<std::int16_t>::min();
        minx = std::numeric_limits<std::int16_t>::max();
        maxx = std::numeric_limits<std::int16_t>::min();

        while (offset < data.size()){
            auto entry = multi_sub_item_t();
            std::copy(data.begin()+offset,data.begin()+offset+2,reinterpret_cast<std::uint8_t*>(&entry.tileid));
            offset+=2 ;
            std::copy(data.begin()+offset,data.begin()+offset+2,reinterpret_cast<std::uint8_t*>(&entry.xoffset));
            offset+=2 ;
            this->minx = std::min(minx,entry.xoffset);
            this->maxx = std::max(maxx,entry.xoffset);
            std::copy(data.begin()+offset,data.begin()+offset+2,reinterpret_cast<std::uint8_t*>(&entry.yoffset));
            offset+=2 ;
            this->miny = std::min(miny,entry.yoffset);
            this->maxy = std::max(maxy,entry.yoffset);
            std::copy(data.begin()+offset,data.begin()+offset+2,reinterpret_cast<std::uint8_t*>(&entry.zoffset));
            offset+=2 ;
            this->minz = std::min(minz,entry.zoffset);
            this->maxz = std::max(maxz,entry.zoffset);
            if (isuop) {
                std::copy(data.begin()+offset,data.begin()+offset+2,reinterpret_cast<std::uint8_t*>(&small_flag));
                offset+=2 ;
                switch (small_flag) {
                    default:
                    case 0:
                        entry.flag =1 ;
                        break;
                    case 256:
                        entry.flag =0x0000000100000001;
                        break;
                    case 257:
                        entry.flag = 0x0000000100000000;
                        break;
                    case 1:
                        entry.flag = 0 ;
                }
                std::copy(data.begin()+offset,data.begin()+offset+4,reinterpret_cast<std::uint8_t*>(&count));
                offset+=4 ;
                for (std::uint32_t j=0; j<count ; j++){
                    std::copy(data.begin()+offset,data.begin()+offset+4,reinterpret_cast<std::uint8_t*>(&cli));
                    offset+=4 ;
                    entry.cliloc.push_back(cli);
                }
            }
            else {
                std::copy(data.begin()+offset,data.begin()+offset+8,reinterpret_cast<std::uint8_t*>(&entry.flag));
                offset+=4 ;
            }
            items.push_back(entry) ;
        }
    }
    //=================================================================================
    auto multi_entry_t::load(const std::filesystem::path &path) ->void {
        auto input = std::ifstream(path) ;
        if (!input.is_open()){
            throw std::runtime_error("Unable to open: "s + path.string());
        }
        auto buffer = std::vector<char>(2048,0);
        while (input.good() && !input.eof()){
            input.getline(buffer.data(),2047);
            if (input.gcount()>0){
                std::string line = buffer.data();
                line = strutil::trim(strutil::strip(line,"//"));
                if (!line.empty()){
                    // We need to ensure not the header line
                    if ((line.at(0)>=48) && (line.at(0)<=57)){
                        items.push_back(multi_sub_item_t(line)) ;
                    }
                }
            }
        }
    }
    
    //=================================================================================
    auto multi_entry_t::description(const std::filesystem::path &path,bool use_hex) ->void{
        auto output = std::ofstream(path.string());
        if (!output.is_open()){
            throw std::runtime_error("Unable to create: "s + path.string());
        }
        // write out header
        output << multi_sub_item_t::header_line<<"\n";
        for (const auto &entry:items){
            output <<entry.description(use_hex)<<"\n";
        }
    }
    //=================================================================================
    auto multi_entry_t::description(std::ofstream &output,bool use_hex) ->void{
        // write out header
        output << multi_sub_item_t::header_line<<"\n";
        for (const auto &entry:items){
            output <<entry.description(use_hex)<<"\n";
        }
    }

    //=================================================================================
    multi_entry_t::multi_entry_t(const std::vector<std::uint8_t> &data, bool isuop):multi_entry_t(){
        load(data,isuop);
    }
    
    
    

}
