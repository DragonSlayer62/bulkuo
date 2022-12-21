//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef bitmap_hpp
#define bitmap_hpp
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <fstream>
#include <utility>
#include <stdexcept>
#include <tuple>
#include <cmath>
//=================================================================================
//=================================================================================
template <class T>
class bitmap_t {
public:
	//==========================================================================
	std::vector<std::uint32_t> palette ;
private:
	std::vector<std::vector<T>> scanline ;
	std::int32_t width;
	std::int32_t height;
	// BMP Header constants
	static constexpr auto filesizeoffset = 2 ;
	static constexpr auto dataoffset = 10 ;
	static constexpr auto bmpheadersize = 14 ;
	static constexpr auto imagesizeoffset = 20 ;
	
	// DIB header constants
	static constexpr auto dibheadersize = 40 ;
	static constexpr auto widthoffset =4 ;
	static constexpr auto heightoffset = 8 ;
	static constexpr auto pixeldepthoffset = 14 ;
	static constexpr auto colornumoffset = 32 ;

	static constexpr auto color32multiplier = static_cast<float>(255.0 / 31.0 );
	static constexpr auto color16multipler = static_cast<float>(1.0/color32multiplier) ;

	//==========================================================================
	constexpr static std::array<std::uint8_t,bmpheadersize> bmpheader{
		'B','M',  	// Signature
		0,0,0,0, 	// total file size
		0,0,0,0, 	// always zero
		14+40,0,0,0	// offset to data
	};
	constexpr static std::array<std::uint8_t,dibheadersize> dibheader {
		dibheadersize,0,0,0,	// Size of header in bytes
		0,0,0,0,				// image width in pixels (signed)
		0,0,0,0,				// image height in pixels (signed)
		1,0,					// color planes (1)
		16,0,					// Bits per pixel
		0,0,0,0,				// compression method (0 = rgb)
		0,0,0,0,				// Image size (size of the raw bitmap data)
		
								//Print resolution of the image,
								//72 DPI × 39.3701 inches per metre yields 2834.6472
		0x13,0xB,0,0,			// horizontal resolution (pixel per miter, signed)
		0x13,0xB,0,0,			// vertical resolution (pixel per miter, signed)
								
		0,0,0,0,				// number of colors in the palette
		0,0,0,0					// number of important colors used;
	};

	//==========================================================================
	auto writeValue(std::uint8_t value, int pixelsize, std::ostream &output) const  ->int {
		auto total = 0 ;
		auto color = palette.at(value) ;
		auto [alpha,red,green,blue] = colorChannels(color, pixelsize);
		switch (pixelsize){
			case 1:
				output.write(reinterpret_cast<char*>(&value),1);;
				total += 1 ;
				break;
			case 16: {
				auto color = std::uint16_t((red<<10) | (green<<5) | blue | (alpha<<15));
				output.write(reinterpret_cast<char*>(&color),sizeof(color));
				total += sizeof(color) ;
				break;
			}
			case 24: {
				output.write(reinterpret_cast<char*>(&blue),1);
				output.write(reinterpret_cast<char*>(&green),1);
				output.write(reinterpret_cast<char*>(&red),1);
				total += 3 ;
				break;
			}
			case 32:
				output.write(reinterpret_cast<char*>(&blue),1);
				output.write(reinterpret_cast<char*>(&green),1);
				output.write(reinterpret_cast<char*>(&red),1);
				output.write(reinterpret_cast<char*>(&alpha),1);
				total += 4 ;
				break;
			default:
				break;
		}
		return total ;
	}
	//==========================================================================
	auto writeValue(std::uint32_t value, int pixelsize, std::ostream &output) const  ->int {
		auto [alpha,red,green,blue] = colorChannels(value, pixelsize);
		auto total = 0 ;
		switch (pixelsize) {
			case 16: {
				auto color = std::uint16_t((red<<10) | (green<<5) | blue | (alpha<<15));
				output.write(reinterpret_cast<char*>(&color),sizeof(color));
				total += sizeof(color) ;
				break;
			}
			case 24: {
				output.write(reinterpret_cast<char*>(&blue),1);
				output.write(reinterpret_cast<char*>(&green),1);
				output.write(reinterpret_cast<char*>(&red),1);
				total += 3 ;
				break;
			}
			case 32:
				output.write(reinterpret_cast<char*>(&blue),1);
				output.write(reinterpret_cast<char*>(&green),1);
				output.write(reinterpret_cast<char*>(&red),1);
				output.write(reinterpret_cast<char*>(&alpha),1);
				total += 4 ;
				break;
			default:
				break;
		}
		return total ;
	}
	//==========================================================================
	auto writeValue(std::uint16_t value,int pixelsize, std::ostream &output) const  -> int {
		auto total = 0 ;
		auto [alpha,red,green,blue] = colorChannels(value, pixelsize);
		switch (pixelsize) {
			case 16: {
				auto color = std::uint16_t((red<<10) | (green<<5) | blue | (alpha<<15));
				output.write(reinterpret_cast<char*>(&color),sizeof(color));
				total += sizeof(color) ;
				break;
			}
			case 24: {
				output.write(reinterpret_cast<char*>(&blue),1);
				output.write(reinterpret_cast<char*>(&green),1);
				output.write(reinterpret_cast<char*>(&red),1);
				total += 3 ;
				break;
			}
			case 32:
				output.write(reinterpret_cast<char*>(&blue),1);
				output.write(reinterpret_cast<char*>(&green),1);
				output.write(reinterpret_cast<char*>(&red),1);
				output.write(reinterpret_cast<char*>(&alpha),1);
				total += 4 ;
				break;
			default:
				break;
		}
		return total ;
	}
	//==========================================================================
	auto writePalette(std::ostream &output) const ->void {
		for (const auto &color:palette){
			auto [alpha,red,green,blue] = colorChannels(color, 24) ;
			output.write(reinterpret_cast<char*>(&blue),1);
			output.write(reinterpret_cast<char*>(&green),1);
			output.write(reinterpret_cast<char*>(&red),1);
			output.write(reinterpret_cast<char*>(&alpha),1);
		}
		auto current = output.tellp() ;
		output.seekp(bmpheadersize+colornumoffset,std::ios::beg);
		auto number = static_cast<std::uint32_t>(palette.size()) ;
		output.write(reinterpret_cast<char*>(&number),4);
		output.seekp(current,std::ios::beg) ;
	}
public:
	//=========================================================================
	auto isGray(T value) ->bool{
		auto mask = T(0xFF);
		auto shift = 8 ;
		if (sizeof(T) == 2) {
			mask = 0x1F;
			shift = 5 ;
		}
		return  (((value>>(2*shift)) & mask) == ((value>>shift) & mask)) && ((value & mask) == ((value>>shift) & mask)) ;
	}
	//=========================================================================
	auto intensify(std::int8_t value, int x, int y) ->void {
		auto mask = 0x1f;
		auto shift = 5 ;
		if (sizeof(T) == 4){
			mask = 0xFF;
			shift = 8 ;
		}
		if (pixel(x,y) != 0){
			
			auto red = ((value>>(shift*2))&mask) + value ; ;
			auto green = ((value>>(shift))&mask) + value ;
			auto blue = (value&mask) + value ;
			pixel(x,y) = static_cast<T>(((red<<shift*2) | (green<<shift) | blue) | (~(mask<<(shift*3))) );
		}
	}
	//=========================================================================
	auto hue(const std::vector<T> &huevalues,bool grayonly) ->bitmap_t<T>{
		auto alpha = T(0x8000) ;
		auto shift = 5 ;
		if (sizeof(T) == 4){
			alpha = 0xFF000000 ;
			shift = 8 ;
		}
		auto mask = T(~alpha) ;
		auto image = bitmap_t<T>(this->width,this->height) ;
		for (auto y=0 ;y<this->height;y++){
			for (auto x=0 ;y<this->width;x++){
				auto value = pixel(x, y) ;
				if (value !=0){
					auto red = ((value&mask)>>(shift*2));
					if (grayonly){
						if (isGray(value)){
							value = huevalues.at(red) ;
						}
					}
					else {
						value = huevalues.at(red) ;
					}
				}
				if (value != 0){
					value |= alpha;
				}
				image.pixel(x,y) = value ;
			}

		}
		return image ;
	}
	//==========================================================================
	static auto colorChannels(std::uint16_t value, int pixelsize) ->std::tuple<std::uint8_t,std::uint8_t,std::uint8_t,std::uint8_t> {
		auto red = std::uint8_t( 0) ;
		auto green =  std::uint8_t( 0) ;
		auto blue =  std::uint8_t( 0) ;
		auto alpha =  std::uint8_t(0) ;
		
		if ((pixelsize == 32) || (pixelsize == 24) ){
			
			red =  static_cast<std::uint8_t>(std::ceil((static_cast<float>(static_cast<std::uint32_t>(value >>10)&0x1F) * color32multiplier )));
			green = static_cast<std::uint8_t>(std::ceil((static_cast<float>(static_cast<std::uint32_t>(value >>5)&0x1F) * color32multiplier )));
			blue = static_cast<std::uint8_t>(std::ceil((static_cast<float>(static_cast<std::uint32_t>(value)&0x1F) * color32multiplier )));
			alpha = ((value&0x8000)!=0?255:0) ;
			if (pixelsize == 24){
				alpha = 0 ;
			}
			
		}
		else if (pixelsize == 16) {
			red =  static_cast<std::uint8_t>(((value >>10)&0x1F) );
			green = static_cast<std::uint8_t>(((value >>5)&0x1F) );
			blue = static_cast<std::uint8_t>(((value)&0x1F) );
			alpha = ((value&0x80)!=0?1:0) ;
		}
		else {
			throw std::runtime_error("Invalid color channel breakout, invalid pixelsize requested.");
		}
		return std::make_tuple(alpha,red,green,blue);
	}
	
	//==========================================================================
	static auto colorChannels(std::uint32_t value,int pixelsize) ->std::tuple<std::uint8_t,std::uint8_t,std::uint8_t,std::uint8_t>{
		auto red = std::uint8_t( 0) ;
		auto green =  std::uint8_t( 0) ;
		auto blue =  std::uint8_t( 0) ;
		auto alpha =  std::uint8_t(0) ;
		if ((pixelsize == 32) || (pixelsize == 24) ){
			
			red = std::uint8_t( (value >>16)&0xFF) ;
			green = std::uint8_t( (value >>8)&0xFF) ;
			blue = std::uint8_t( (value )&0xFF) ;
			alpha = std::uint8_t( (value>>24 )&0xFF) ;
			if (pixelsize == 24){
				alpha = 0 ;
			}

		}
		else if (pixelsize == 16) {
			red = std::uint8_t( std::floor( static_cast<float>(static_cast<std::uint32_t>(value >>16)&0xFF) * color16multipler)) ;
			green = std::uint8_t( std::floor( static_cast<float>(static_cast<std::uint32_t>(value >>8)&0xFF) * color16multipler)) ;
			blue = std::uint8_t( std::floor( static_cast<float>(static_cast<std::uint32_t>(value) &0xFF) * color16multipler)) ;
			alpha = (std::uint8_t( (value>>24 )&0xFF)==255?1:0) ;
		}
		else {
			throw std::runtime_error("Invalid color channel breakout, invalid pixelsize requested.");
		}
		return std::make_tuple(alpha,red,green,blue);
	}
	//==========================================================================
	static auto convertColor(std::uint16_t color) ->std::uint32_t {
		auto [alpha,red,green,blue] = colorChannels(color, 32) ;
		auto rcolor = static_cast<std::uint32_t>(alpha) << 24 ;
		rcolor |= (static_cast<std::uint32_t>(red) << 16) ;
		rcolor |= (static_cast<std::uint32_t>(green) << 8) ;
		rcolor |= (static_cast<std::uint32_t>(blue) ) ;
		return rcolor ;
	}
	//==========================================================================
	static auto convertColor(std::uint32_t color) ->std::uint16_t {
		auto [alpha,red,green,blue] = colorChannels(color, 16) ;
		auto rcolor = static_cast<std::uint16_t>(alpha) << 15 ;
		rcolor |= (static_cast<std::uint16_t>(red) << 10) ;
		rcolor |= (static_cast<std::uint16_t>(green) << 5) ;
		rcolor |= (static_cast<std::uint16_t>(blue) ) ;
		return rcolor ;
	}
	//==========================================================================
	static auto readPalette(std::istream &input,std::uint32_t number) ->std::vector<std::uint32_t> {
		auto temp  = std::vector<std::uint32_t>();
		if (number > 0){
			temp  = std::vector<std::uint32_t>(number,0) ;
			auto color = std::uint32_t(0) ;
			for (std::uint32_t j=0;j<number;j++){
				input.read(reinterpret_cast<char*>(&color),4);
				temp[j] = color ;
			}
		}
		return temp ;
	}
	

	
	//==========================================================================
	bitmap_t<T>(int width=0,int height=0):width(width),height(height){
		this->size(width,height) ;
	}
	//============================================================================
	auto size() const ->std::pair<std::int32_t,std::int32_t> {
		return std::make_pair(width,height) ;
	}
	//============================================================================
	auto size(std::int32_t width, std::int32_t height) ->void {
		this->width = width ;
		this->height = height ;
		auto line = std::vector<T>(width,0) ;
		scanline = std::vector<std::vector<T>>(height,line);
	}
	//============================================================================
	auto empty() const ->bool {
		return scanline.empty();
	}
	//============================================================================
	auto pixeldepth() const -> int {
		return sizeof(T) ;
	}
	//==========================================================================
	auto fill(T color) ->bitmap_t<T>& {
		std::fill(scanline.begin(),scanline.end(),std::vector<T>(width,color));
		return *this ;
	}
	//==========================================================================
	auto invert() ->bitmap_t<T>& {
		std::reverse(scanline.begin(), scanline.end());
		return *this ;
	}
	//==========================================================================
	auto mirror() ->bitmap_t<T> {
		for (auto y=0 ; y< height; ++y) {
			std::reverse(scanline[y].begin(),scanline[y].end());
		}
		return *this ;
	}
	//==========================================================================
	auto pixel(int x, int y) const -> const T&{
		if (x>=width || y>=height) {
			throw std::runtime_error("bitmap_t::pixel - Tried to access beyond image size.");
		}
		return scanline[y][x] ;
	}
	//==========================================================================
	auto pixel(int x, int y)  ->  T&{
		if (x>=width || y>=height) {
			throw std::runtime_error("bitmap_t::pixel - Tried to access beyond image size.");
		}
		return scanline[y][x] ;
	}
	
	//==========================================================================
	auto saveToBMP(std::ostream &output, int pixelsize=24,bool inverted=false) const ->void{
		if (!output.good()){
			throw std::runtime_error("saveToBMP - Stream not good.");
		}
		// Write the bmp header
		output.write(reinterpret_cast<const char*>(bmpheader.data()),bmpheader.size());
		// Write the dib header
		output.write(reinterpret_cast<const char*>(dibheader.data()),dibheader.size());

		// Write the palette
		writePalette(output);

		// Write the data
		auto temp = scanline ;
		if (inverted){
			std::reverse(temp.begin(), temp.end());
		}
		auto bytesize = pixelsize/8 ;
		auto mod = (width*bytesize)%4 ;
		
		auto pad = 4-(mod!=0?mod:4);
		//auto pad = bytesize/4 + (4-(bytesize%4)) ;
		auto padvalue = std::vector<char>(pad,0);
		auto total = 0 ;
		
		auto datastart = static_cast<std::uint32_t>(output.tellp());
		for (auto y=0 ; y<height;y++){
			for (auto x = 0 ; x<width;x++){
				total += writeValue(temp[(height-1)-y][x], pixelsize, output);
			}
			if (!padvalue.empty()){
				output.write(padvalue.data(),padvalue.size());
				total +=  static_cast<int>(padvalue.size() );
			}
		}
		// adjust parameters
		auto filesize = static_cast<std::uint32_t>(output.tellp() );
		output.seekp(filesizeoffset,std::ios::beg);
		output.write(reinterpret_cast<char*>(&filesize),4);
		output.seekp(dataoffset,std::ios::beg);
		output.write(reinterpret_cast<char*>(&datastart),4);
		output.seekp(bmpheadersize + widthoffset,std::ios::beg);
		output.write(reinterpret_cast<const char*>(&width),4);
		output.write(reinterpret_cast<const char*>(&height),4);

		auto one = std::uint16_t(1) ;
		output.write(reinterpret_cast<const char*>(&one),2);
		output.write(reinterpret_cast<char*>(&pixelsize),2);
		output.seekp(bmpheadersize + imagesizeoffset,std::ios::beg);
		output.write(reinterpret_cast<char*>(&total),4);

	}
	//=========================================================================
	static auto indexFor(std::uint32_t color, std::vector<std::uint32_t> &palette) -> std::uint8_t {
		auto iter = std::find_if(palette.begin(),palette.end(),[color](const std::uint32_t &value){
			return color == value ;
		});
		if (iter != palette.end()){
			return static_cast<std::uint8_t>(std::distance(palette.begin(), iter));
		}
		throw std::runtime_error("indexFor - Color not present in palette.");
	}
	//==========================================================================
	static auto fromBMP(std::istream &input) ->bitmap_t<T> {
		if (!input.good()){
			throw std::runtime_error("fromBMP - input stream is not good.");
		}
		auto byte = char(0) ;
		input.read(&byte,1) ;
		if (byte != 'B'){
			throw std::runtime_error("fromBMP - input stream not a BMP file.");
		}
		input.read(&byte,1) ;
		if (byte != 'M'){
			throw std::runtime_error("fromBMP - input stream not a BMP file.");
		}
		input.seekg(8,std::ios::cur) ;
		auto offsetToData = std::uint32_t(0) ;
		input.read(reinterpret_cast<char*>(&offsetToData),4) ;
		// Now read in the DIB header
		auto dibHeaderSize = std::uint32_t(0) ;
		input.read(reinterpret_cast<char*>(&dibHeaderSize),4) ;
		auto width = std::int32_t(0) ;
		auto height = std::int32_t(0);
		input.read(reinterpret_cast<char*>(&width),4) ;
		input.read(reinterpret_cast<char*>(&height),4) ;
		auto pixelsize = std::uint16_t(0) ;
		input.seekg(2,std::ios::cur) ;
		input.read(reinterpret_cast<char*>(&pixelsize),2) ;
		auto compression = std::uint32_t(0);
		input.read(reinterpret_cast<char*>(&compression),4);
		if (compression != 0){
			throw std::runtime_error("fromBMP - Invalid stream format, compressed BMP data not supported.");
		}
		input.seekg(12,std::ios::cur) ;
		auto palettesize = std::uint32_t(0) ;
		input.read(reinterpret_cast<char*>(&palettesize),4) ;
		
		// seek past the dibheader
		input.seekg(14+dibHeaderSize,std::ios::beg) ;
		auto image = bitmap_t<T>(width,height) ;
		// Read in the palette ;
	
		auto palette  = readPalette(input, palettesize);
		if ((pixelsize==8) && (sizeof(T) ==1)){
			image.palette = palette ;
		}
		// Seek to the data
		input.seekg(offsetToData,std::ios::beg) ;
		
		auto pad = ((pixelsize/8)*width)%4 ;
		pad = (pad!=0?4-pad:0) ;
		auto padvalue = std::vector<char>(pad,0) ;
		
		// In case we need to make a palette
		auto createdPalette = std::vector<std::uint32_t>() ;
		for (auto y=0;y<height;y++){
			if (!input.good()){
				throw std::runtime_error("fromBMP - Invalid stream during data read.");
			}
			for (auto x=0;x<width;x++){
				switch(sizeof(T)){
					case 1:{
						switch(pixelsize) {
							case 8:{
								auto color = std::uint8_t(0) ;
								input.read(reinterpret_cast<char*>(&color),sizeof(color));
								image.pixel(x,(height-1)-y) = color ;
								break;
							}
							case 16:{
								auto color = std::uint16_t(0) ;
								input.read(reinterpret_cast<char*>(&color),sizeof(color));
								auto value = convertColor(color);
								auto index = std::uint8_t(0);
								try {
									index = indexFor(value, createdPalette);
								}
								catch(...){
									createdPalette.push_back(value);
									index = static_cast<std::uint8_t>(createdPalette.size()-1);
								}
								image.pixel(x,(height-1)-y) = index ;
								
								break;
							}
							case 24:{
								auto channel = std::uint8_t(0) ;
								auto color = std::uint32_t(0) ;
								input.read(reinterpret_cast<char*>(&channel),sizeof(channel));
								color |= static_cast<std::uint32_t>(channel) ;
								input.read(reinterpret_cast<char*>(&channel),sizeof(channel));
								color |=(static_cast<std::uint32_t>(channel)<<8) ;
								input.read(reinterpret_cast<char*>(&channel),sizeof(channel));
								color |=(static_cast<std::uint32_t>(channel)<<16) ;
								color |=(255 << 24) ;
								auto index = std::uint8_t(0);
								try {
									index = indexFor(color, createdPalette);
								}
								catch(...){
									createdPalette.push_back(color);
									index = static_cast<std::uint8_t>(createdPalette.size()-1);
								}
								image.pixel(x,(height-1)-y) = index ;
								break;
							}
							case 32:{
								auto color = std::uint32_t(0) ;
								input.read(reinterpret_cast<char*>(&color),sizeof(color));
								auto value = color;
								auto index = std::uint8_t(0);
								try {
									index = indexFor(value, createdPalette);
								}
								catch(...){
									createdPalette.push_back(value);
									index = static_cast<std::uint8_t>(createdPalette.size()-1);
								}
								image.pixel(x,(height-1)-y) = index ;
								break;
							}
							default:
								throw std::runtime_error("fromBMP - Invalid pixel size for input BMP.");
						}
						break;
					}
					case 2:{
						switch(pixelsize) {
							case 8:{
								auto color = std::uint8_t(0) ;
								input.read(reinterpret_cast<char*>(&color),sizeof(color));
								auto value = palette[color] ;
								image.pixel(x,(height-1)-y) = convertColor(value);
								break;
							}
							case 16:{
								auto color = std::uint16_t(0) ;
								input.read(reinterpret_cast<char*>(&color),sizeof(color));
								image.pixel(x,(height-1)-y) = color ;
								break;
							}
							case 24:{
								auto channel = std::uint8_t(0) ;
								auto color = std::uint32_t(0) ;
								input.read(reinterpret_cast<char*>(&channel),sizeof(channel));
								color |= static_cast<std::uint32_t>(channel) ;
								input.read(reinterpret_cast<char*>(&channel),sizeof(channel));
								color |=(static_cast<std::uint32_t>(channel)<<8) ;
								input.read(reinterpret_cast<char*>(&channel),sizeof(channel));
								color |=(static_cast<std::uint32_t>(channel)<<16) ;
								color |=(255 << 24) ;
								image.pixel(x,(height-1)-y) = convertColor(color);
								break;
							}
							case 32:{
								auto color = std::uint32_t(0) ;
								input.read(reinterpret_cast<char*>(&color),sizeof(color));
								image.pixel(x,(height-1)-y) = convertColor(color) ;
								break;
							}
							default:
								throw std::runtime_error("fromBMP - Invalid pixel size for input BMP.");
						}

						break;
					}
					case 4:{
						switch(pixelsize) {
							case 8:{
								auto color = std::uint8_t(0) ;
								input.read(reinterpret_cast<char*>(&color),sizeof(color));
								image.pixel(x,(height-1)-y) = color ;
								break;
							}
							case 16:{
								auto color = std::uint16_t(0) ;
								input.read(reinterpret_cast<char*>(&color),sizeof(color));
								;
								image.pixel(x,(height-1)-y) = convertColor(color) ;
								break;
							}
							case 24:{
								auto channel = std::uint8_t(0) ;
								auto color = std::uint32_t(0) ;
								input.read(reinterpret_cast<char*>(&channel),sizeof(channel));
								color |=channel ;
								input.read(reinterpret_cast<char*>(&channel),sizeof(channel));
								color |=(channel<<8) ;
								input.read(reinterpret_cast<char*>(&channel),sizeof(channel));
								color |=(channel<<16) ;
								color |=(255 << 24) ;
								image.pixel(x,(height-1)-y) = color ;
								break;
							}
							case 32:{
								auto color = std::uint32_t(0) ;
								input.read(reinterpret_cast<char*>(&color),sizeof(color));
								break;
							}
							default:
								throw std::runtime_error("fromBMP - Invalid pixel size for input BMP.");
						}

						break;
					}
					default:
						throw std::runtime_error("fromBMP - Invalid pixel size for destination BMP.");
						
				}
			}
			input.read(padvalue.data(),padvalue.size());
		}
		if (!createdPalette.empty()){
			if ((createdPalette.size() >256)&& (sizeof(T)==1)){
				throw std::runtime_error("fromBMP - Created paletted is to large.");
			}
			image.palette = createdPalette;
		}
		return image;
	}
};

#endif /* bitmap_hpp */
