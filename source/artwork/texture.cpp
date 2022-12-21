//Copyright © 2022 Charles Kerr. All rights reserved.

#include "texture.hpp"

#include <iostream>
#include <algorithm>
#include <stdexcept>

using namespace std::string_literals;

//=================================================================================
//=================================================================================
auto bitmapForTexture(const std::vector<std::uint8_t> &data) ->bitmap_t<std::uint16_t>  {
	auto width = 64 ;
	if (data.size() == 32768){
		width = 128;
	}
	auto color = reinterpret_cast<const std::uint16_t *>(data.data()) ;
	auto image = bitmap_t<std::uint16_t>(width,width) ;
	for (auto y=0 ; y < width ; y++){
		for (auto x=0 ; x< width ; x++){
			image.pixel(x,y) = (((*color)&0x7fff)!=0 ?(*color) | 0x8000:0);
			color++ ;
		}
	}
	return image ;
}

//=================================================================================
auto dataForTexture(const bitmap_t<std::uint16_t> &image) ->std::vector<std::uint8_t> {
	auto [width,height] = image.size() ;
	auto data = std::vector<std::uint8_t>(width*height*2,0);
	auto color = reinterpret_cast<std::uint16_t *>(data.data()) ;

	for (auto y=0; y<height;y++){
		for (auto x=0; x<width;x++){
			*color = image.pixel(x,y) & 0x7FFF;
			color++ ;
		}
	}
	return data ;
}
