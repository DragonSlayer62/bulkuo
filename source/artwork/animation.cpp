//Copyright © 2022 Charles Kerr. All rights reserved.

#include "animation.hpp"

#include <iostream>
#include <stdexcept>
#include <set>

using namespace std::string_literals;

//===========================================================================================================
auto decipherAnimData(const std::vector<std::uint8_t> &data) ->std::vector<std::tuple<std::int16_t,std::int16_t,bitmap_t<std::uint16_t>>> {
	auto rvalue = std::vector<std::tuple<std::int16_t,std::int16_t,bitmap_t<std::uint16_t>>>();
	auto palette = std::vector<std::uint16_t>(256,0);
	auto ptr = reinterpret_cast<const std::uint16_t*>(data.data());
	for (auto j=0 ; j<256;j++){
		palette[j] = ptr[j];
	}
	auto dwordptr = reinterpret_cast<const std::uint32_t*>(data.data()+512);
	auto framecount = *dwordptr ;
	auto offsets = std::vector<std::uint32_t>(framecount,0) ;
	for (std::uint32_t j=0; j<framecount;j++){
		offsets[j] = dwordptr[j+1] ;
		//std::cout <<"Offset: "<<offsets[j]<<std::endl;
	}
	auto byteoffset = static_cast<std::uint32_t>(512) ;
	for (size_t j=0 ; j<offsets.size();j++) {
		auto size = static_cast<std::uint32_t>(offsets.size()) - (byteoffset+offsets[j]);
		if (j<(offsets.size()-1)){
			size = offsets[j+1] - offsets[j];
		}
		rvalue.push_back(decipherAnimFrame(data.data()+byteoffset+offsets[j], size,palette));
	}
	return rvalue ;
}

//===========================================================================================================
auto decipherAnimFrame(const std::uint8_t *data,std::uint32_t length,const std::vector<std::uint16_t> &palette) ->std::tuple<std::int16_t , std::int16_t , bitmap_t<std::uint16_t>> {
	//std::cout <<"data size: "<<length << std::endl;
	
	constexpr auto doubleXor = (0x200 << 22) | (0x200 << 12) ;
	
	auto wordptr = reinterpret_cast<const std::int16_t*>(data) ;
	auto xCenter = static_cast<int>(*wordptr) ;
	//std::cout <<"xCenter: "<<xCenter<< std::endl;

	wordptr++ ;
	auto yCenter= static_cast<int>(*wordptr) ;
	//std::cout <<"yCenter: "<<yCenter<< std::endl;

	wordptr++ ;
	auto width= static_cast<int>(*wordptr );
	//std::cout <<"Width: "<<width<< std::endl;

	wordptr++ ;
	auto height=static_cast<int>( *wordptr) ;
	//std::cout <<"Height: "<<height<< std::endl;
	if ((height<=0) | (width <=0)){
		return std::make_tuple(xCenter,yCenter,bitmap_t<std::uint16_t>());
		
	}
	auto xBase = xCenter - 0x200;
	auto yBase = (yCenter + height) - 0x200;
	auto image = bitmap_t<std::uint16_t>(width,height);
	auto y = yBase;
	auto x = xBase ;
	auto header = std::uint32_t(0) ;

	std::uint32_t streamsize = length - 8 ;
	auto streamdata = reinterpret_cast<const std::uint8_t*>(data+length-streamsize);
	std::copy(streamdata,streamdata+4,reinterpret_cast<std::uint8_t*>(&header));
	while (header != 0x7FFF7FFF){
		header ^= doubleXor ;
		auto deltay = ((header>>12) & 0x3ff) ;
		auto deltax = (header>>22) &0x3ff ;
		auto runlength = header&0xFFF ;
		streamdata +=4 ;
		for (std::uint32_t j=0 ; j < runlength;j++){
			std::uint16_t color = (palette[*streamdata]==0?0:palette[*streamdata]|0x80);
			streamdata++;
			//std::cout <<"Update: " <<x+deltax+j<<" , "<< y+deltay<<std::endl;
			image.pixel(x+deltax+j, y+deltay)= color ;
		}
		std::copy(streamdata,streamdata+4,reinterpret_cast<std::uint8_t*>(&header));
	}
	return std::make_tuple(xCenter,yCenter,image) ;
}

//===========================================================================================================
auto paletteForAnim(const bitmap_t<std::uint16_t> &image,std::set<std::uint16_t> &current) ->void{
	auto [width,height] = image.size() ;
	for (std::int32_t y = 0 ;y<height;y++){
		for (std::int32_t x=0 ; x<width;x++){
			current.insert(image.pixel(x, y) & 0x7fff);
		}
	}
}
//===========================================================================================================
auto indexForAnimPalette(std::uint16_t color,const std::vector<std::uint16_t> &palette) ->std::uint8_t {
	auto iter = std::find_if(palette.begin(),palette.end(),[color](std::uint16_t value){
		return (color&0x7FFF) == value ;
	});
	if (iter != palette.end()){
		return static_cast<std::uint8_t>(std::distance(palette.begin(), iter)) ;
	}
	throw std::runtime_error("Color not found in animation palette.");
}
//===========================================================================================================
auto dataForAnimFrameLine(const bitmap_t<std::uint16_t> &image, std::int32_t y,const std::vector<std::uint16_t> &palette)->std::vector<std::pair<std::int32_t,std::vector<std::uint8_t>>> {
	
	auto rvalue = std::vector<std::pair<std::int32_t,std::vector<std::uint8_t>>>() ;
	auto [width,height] = image.size() ;
	auto lastcolor = std::uint16_t(0) ;
	auto runx = -1 ;
	auto colors = std::vector<std::uint8_t>() ;
	for (auto x=0 ; x<width;x++){
		auto color = image.pixel(x, y) &0x7FFF ;
		if (color == 0){
			if (lastcolor != 0){
				rvalue.push_back(std::make_pair(runx, colors));
				colors.clear() ;
				runx = -1 ;
			}
			lastcolor = color ;

		}
		else {
			colors.push_back(indexForAnimPalette(color, palette));
			lastcolor = color ;
			if (runx == -1){
				runx = x ;
			}
		}
	}
	if (runx != -1){
		rvalue.push_back(std::make_pair(runx,colors));
	}
	return rvalue ;
}
