//Copyright © 2022 Charles Kerr. All rights reserved.

#include "gump.hpp"

#include <iostream>
#include <stdexcept>

using namespace std::string_literals;

//=================================================================================

//=================================================================================
// internal routines used during the conversion
//=================================================================================

//=================================================================================
auto dataForGumpLine(const bitmap_t<std::uint16_t> &image, int y) ->std::vector<std::uint16_t> {
	auto temp = std::vector<std::uint16_t>() ;
	auto [width,height] = image.size() ;
	auto run = 0 ;
	auto color = std::uint16_t(0) ;
	for (auto j=0;j<width;j++){
		if (j == 0 ){
			color = image.pixel(j,y) & 0x7FFF;
			run = 1 ;
		}
		else {
			if (color == (image.pixel(j,y) & 0x7FFF)){
				run++ ;
			}
			else {
				temp.push_back(color) ;
				temp.push_back(run) ;
				run = 1 ;
				color =  (image.pixel(j,y) & 0x7FFF) ;
			}
		}
	}
	temp.push_back(color);
	temp.push_back(run) ;

	return temp ;
}

//=================================================================================
// Public routines
//=================================================================================

//==============================================================================
//std::uint32_t datalookup[height]  - points to the offset for the Scanline in bytes
//The scanline is stored as
//std::uint16_t color
//std::uint16_t run
//=================================================================================
auto bitmapForGump(const std::vector<std::uint8_t> &data) ->bitmap_t<std::uint16_t> {
	if (data.size()<=8) {
		throw std::runtime_error("artForGump - Invalid data size.");
	}
	
	auto width = std::uint32_t(0);
	auto height = std::uint32_t(0) ;
	std::copy(data.begin(),data.begin()+4,reinterpret_cast<std::uint8_t*>(&width));
	std::copy(data.begin()+4,data.begin()+8,reinterpret_cast<std::uint8_t*>(&height));

	auto image = bitmap_t<std::uint16_t>(width,height) ;
	auto offsets = std::vector<std::uint32_t>(height,0) ;
	auto ptroffset = reinterpret_cast<const std::uint32_t*>(data.data()+8);
	for (std::uint32_t j=0 ; j< height; j++){
		offsets[j] = *(ptroffset+j) ;
	}
	auto blocksize = 0 ;
	auto run = std::uint16_t(0) ;
	auto color = std::uint16_t(0) ;
	
	for (std::uint32_t y = 0 ; y<height;y++){
		if (y < height-1){
			blocksize = offsets[y+1] - offsets[y] ;
		}
		else {
			blocksize = static_cast<std::uint32_t>((data.size()-8)/4) - offsets[y];
		}
		auto pos =  (offsets[y] * 4) + 8 ;
		
		auto x = 0 ;
		for (auto entry = 0 ; entry < blocksize ; entry++){
			std::copy(data.data()+pos,data.data()+pos+2,reinterpret_cast<char*>(&color));
			std::copy(data.data()+pos+2,data.data()+pos+4,reinterpret_cast<char*>(&run));
			pos += 4 ;
			for (auto j = 0 ; j < run ; j++){
				image.pixel(x, y) = (((color)&0x7fff)!=0 ?(color) | 0x8000:0);
				x++ ;
			}
		}
	}
	return image ;
}

//=================================================================================
auto dataForGump(const bitmap_t<std::uint16_t> &image) ->std::vector<std::uint8_t> {
	auto [width,height] = image.size() ;
	auto lines = std::vector<std::vector<std::uint16_t>>(height);
	// Gather up all the lines of the gump
	for (auto y = 0 ;y < height;y++){
		lines[y] = dataForGumpLine(image, y);
	}
	// Now lets create the offset table
	auto offsets = std::vector<std::uint32_t>(height,0) ;
	auto base = height  ;
	for (auto j=0 ; j<height;j++){
		offsets[j] = base ;
		base += static_cast<std::int32_t>(lines[j].size())/2 ;
	}
	//
	auto sizes = std::vector<std::uint32_t>(2,0) ;
	sizes[0]= width ;
	sizes[1] = height;
	
	auto total = offsets.size()*4 + sizes.size()*4 ;
	for (const auto &line:lines){
		total += line.size()*2 ;
	}
	
	auto data = std::vector<std::uint8_t>(total,0) ;
	// copy the sizes
	std::copy(reinterpret_cast<std::uint8_t*>(sizes.data()),reinterpret_cast<std::uint8_t*>(sizes.data())+8,data.begin() );
	auto cpoffset = 8 ;
	// Copy the offsets
	std::copy(reinterpret_cast<std::uint8_t*>(offsets.data()),reinterpret_cast<std::uint8_t*>(offsets.data())+offsets.size()*4,data.begin()+cpoffset );
	cpoffset += static_cast<std::int32_t>(offsets.size())*4 ;
	for (const auto &line:lines){
		std::copy(reinterpret_cast<const std::uint8_t*>(line.data()),reinterpret_cast<const std::uint8_t*>(line.data())+line.size()*2,data.begin()+cpoffset );
		cpoffset += static_cast<std::int32_t>(line.size())*2;
	}
	return data ;
}
