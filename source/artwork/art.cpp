//Copyright © 2022 Charles Kerr. All rights reserved.

#include "art.hpp"

#include <iostream>
#include <map>

using namespace std::string_literals;

//=================================================================================
// internal routines used during the conversion
//=================================================================================

//===================================================================================================
auto lineDataForItem(const bitmap_t<std::uint16_t> &image , int y) ->std::vector<std::uint16_t> {
	auto [width,height] = image.size() ;
	auto line = std::vector<std::uint16_t>();
	auto xoffset = std::uint16_t(0);
	auto run = std::uint16_t(0);
	auto oldcolor = std::uint16_t(0) ;
	auto colors = std::vector<std::uint16_t>() ;
	for (std::uint16_t x = 0 ; x<width;x++){
		auto color = image.pixel(x,y)&0x7FFF ;
		if (color == 0) {
			if (oldcolor!=0){
				line.push_back(xoffset);
				line.push_back(run);
				for (auto value:colors){
					line.push_back(value);
				}
				oldcolor=0 ;
				xoffset =0  ;
			}
			xoffset += 1;
			run = 0 ;
			colors.clear();
		}
		else {
			run += 1 ;
			colors.push_back(color) ;
			oldcolor = color ;
		}
	}
	if (!colors.empty()){
		line.push_back(xoffset);
		line.push_back(run) ;
		for (auto value:colors){
			line.push_back(value);
		}
	}
	line.push_back(0);
	line.push_back(0);
	if (line.size()%2==1){
		line.push_back(0);
	}
	return line;
}

//=================================================================================
auto dataLinesForItem(const bitmap_t<std::uint16_t> &image,std::vector<std::vector<std::uint16_t>> &data) ->std::map<int,int> {
	auto [width,height] = image.size() ;
	auto indicies = std::map<int,int>() ;
	data.clear() ;
	for (auto y = 0;y<height;y++){
		auto line = lineDataForItem(image, y);
		// See if this is all ready in the data
		auto iter = std::find_if(data.begin(),data.end(),[&line](const std::vector<std::uint16_t> &value){
			return line == value ;
		});
		if (iter != data.end()){
			// This line is all ready in our data
			auto index = static_cast<int>(std::distance(data.begin(), iter)) ;
			indicies.insert_or_assign(y, index);
		}
		else {
			// It isn't in our data
			data.push_back(line);
			indicies.insert_or_assign(y, static_cast<int>(data.size()-1));
		}
	}
	return indicies;
}

//======================================================================================
// Public routines
//=======================================================================================

//=================================================================================
auto bitmapForTerrain(const std::vector<std::uint8_t> &data) ->bitmap_t<std::uint16_t> {
	auto image = bitmap_t<std::uint16_t>(44,44) ;
	auto color = reinterpret_cast<const std::uint16_t*>(data.data());
	auto run = 2 ;
	auto xloc = 21 ;
	for (auto height = 0 ; height < 22;height++){
		for (auto offset = 0 ; offset < run ; offset++){
			image.pixel(xloc+offset, height) = (((*color)&0x7fff)!=0 ?(*color) | 0x8000:0) ;
			color++ ;
		}
		xloc-- ;
		run +=2 ;
	}
	run = 44 ;
	xloc = 0 ;

	for (auto height = 22 ; height < 44;height++){
		for (auto offset = 0 ; offset < run ; offset++){
			image.pixel(xloc+offset, height) = (((*color)&0x7fff)!=0 ?(*color) | 0x8000:0);
			color++ ;
		}
		xloc++ ;
		run -=2 ;
	}
	return image ;
}
#include "strutil.hpp"
//=================================================================================
auto dataForTerrain(const bitmap_t<std::uint16_t> &image) ->std::vector<std::uint8_t> {
	auto data = std::vector<std::uint8_t>(2024,0) ;
	auto color = reinterpret_cast<std::uint16_t *>(data.data());
	auto run = 2 ;
	auto xloc = 21 ;
	for (auto height = 0 ; height < 22;height++){
		for (auto offset = 0 ; offset < run ; offset++){
 //           std::cout <<"Pixel: "<<xloc+offset <<" , " << height<<" - Color: "<<strutil::ntos(image.pixel(xloc+offset, height),strutil::radix_t::hex,true,4 ) <<std::endl;
			*color = image.pixel(xloc+offset, height)  ;
			color++ ;
		}
        //std::cout <<std::endl;
		xloc-- ;
		run +=2 ;
	}
	run = 44 ;
	xloc = 0 ;

	for (auto height = 22 ; height < 44;height++){
		for (auto offset = 0 ; offset < run ; offset++){
            //std::cout <<"Pixel: "<<xloc+offset <<" , " << height<<std::endl;
			*color = image.pixel(xloc+offset, height)  & 0x7FFF;
			color++ ;
		}
        //std::cout <<std::endl;
		xloc++ ;
		run -=2 ;
	}
	return data ;
}
//==============================================================================
// std::uint32_t unknown (possibly "extra" from idx format?)
// std::uint16_t width ;
// std::uint16_t height ;
// std::uint16_t scanlineoffset[height]
// a set of data, in the format
// std::uint16_t xoffset
// std::uint16_t run
// std::uint16_t colors[run]
//
//=================================================================================
auto bitmapForItem(const std::vector<std::uint8_t> &data) ->bitmap_t<std::uint16_t> {
	if (data.size()>8){
		auto unknown = std::uint32_t(0);
		std::copy(data.begin(),data.begin()+4,reinterpret_cast<std::uint8_t*>(&unknown));
		auto wdata = reinterpret_cast<const std::uint16_t*>(data.data()+4);
		auto width = *wdata ;
		wdata++ ;
		if ((width >0) && (width <1024)) {
			auto height = *wdata ;
			wdata++ ;
			if ((height>0) && (height < 1024)){
				auto image = bitmap_t<std::uint16_t>(width,height) ;
				
				//auto tableoffset = wdata ;
				//auto dataoffset = tableoffset + height ;
				auto x = 0 ;
				auto y = 0 ;
				auto offset = reinterpret_cast<const std::uint16_t *>(data.data()+8+(height*2) + wdata[y]*2);
				while (y < height) {
					auto xoff = *offset ;
					offset++;
					auto run = *offset ;
					offset++;
					if ((xoff + run) >= 2048) {
						break;
					}
					else if ((xoff+run) != 0){
						x += xoff ;
						for (auto j= 0 ; j< run; j++){
							auto color = *offset ;
							image.pixel(x+j, y) =(((color)&0x7fff)!=0 ?(color) | 0x8000:0);
							offset++ ;
						}
						x+= run ;
					}
					else {
						x = 0 ;
						y++ ;
						if (y < height){
							offset = reinterpret_cast<const std::uint16_t *>(data.data()+8+(height*2) + wdata[y]*2);
						}
					}
				}
				return image ;
			}
			else {
				throw std::runtime_error("Image not avaliable, invalid height.");
			}
		}
		else {
			throw std::runtime_error("Image not avaliable, invalid width.");
		}
	}
	else {
		throw std::runtime_error("Not sufficent data for image.");
	}
	
}
//===============================================================================
auto dataForItem(const bitmap_t<std::uint16_t> &image) -> std::vector<uint8_t> {
	auto [width,height] = image.size();
	auto data = std::vector<std::vector<std::uint16_t>>() ;
	auto indicies = dataLinesForItem(image, data);
	// Now create an offset vector to match data
	std::vector<std::uint16_t> offsets(data.size(),0) ;
	for (auto j=1 ; j< data.size();j++){
		offsets[j] = static_cast<std::uint16_t>(data[j-1].size()) + offsets[j-1] ;
	}
	auto datasize = 0 ;
	for (const auto &line:data){
		datasize += static_cast<int>(line.size())*2 ;
	}

	// Now we create the vector of data
	auto rvalue = std::vector<uint8_t>(datasize + 4 + 4 + height*2,0); // datasize, 4 byte unknown at beginng, 4 byte width/height, height*2 bytes for offsettable.
	auto startoffset = 4 ;
	std::copy(reinterpret_cast<std::uint8_t*>(&width),reinterpret_cast<std::uint8_t*>(&width)+2,rvalue.begin()+startoffset);
	startoffset += 2 ;
	
	std::copy(reinterpret_cast<std::uint8_t*>(&height),reinterpret_cast<std::uint8_t*>(&height)+2,rvalue.begin()+startoffset);
	startoffset += 2 ;
	for (auto y=0 ; y<height;y++){
		auto value = offsets.at(indicies.at(y)) ;
		std::copy(reinterpret_cast<std::uint8_t*>(&value),reinterpret_cast<std::uint8_t*>(&value)+2,rvalue.begin()+startoffset);
		startoffset += 2 ;
	}
	// And now we copy the data
	for (const auto &line:data){
		std::copy(reinterpret_cast<const std::uint8_t*>(line.data()),reinterpret_cast<const std::uint8_t*>(line.data())+(line.size()*2),rvalue.begin()+startoffset);
		startoffset += static_cast<int>(line.size()*2) ;
	}
	
	return rvalue;
}
