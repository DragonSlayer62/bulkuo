//

#include "animation.hpp"
#include <stdexcept>
#include <set>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <sstream>

#include "strutil.hpp"

using namespace std::string_literals;


//=================================================================================
// palette_t
//=================================================================================
//=================================================================================
palette_t::palette_t(){
    colors.resize(256,0);
}
//=================================================================================
palette_t::palette_t(const std::vector<anim_frame> &frames):palette_t(){
    auto setcolor = std::set<std::uint16_t>() ;
    for (const auto &frame:frames){
        if (!frame.image.empty()){
            auto [width,height] = frame.image.size();
            for (auto y=0; y<height;y++){
                for (auto x=0;x<width;x++){
                    setcolor.insert(frame.image.pixel(x,y)) ;
                }
            }
        }
    }
    if (setcolor.size()>256) {
        throw std::runtime_error("Too many colors for palette");
    }
    auto offset = 0 ;
    auto current = setcolor.begin();
    while (current!= setcolor.end()){
        colors.at(offset) = *current ;
        offset++;
        current++;
    }
    
}

//=================================================================================
palette_t::palette_t(const std::vector<bitmap_t<std::uint16_t>> &images):palette_t(){
    auto setcolor = std::set<std::uint16_t>() ;
    for (const auto &image:images){
        if (!image.empty()){
            auto [width,height] = image.size();
            for (auto y=0; y<height;y++){
                for (auto x=0;x<width;x++){
                    setcolor.insert(image.pixel(x,y)&0x7FFF) ;
                }
            }
        }
    }
    if (setcolor.size()>256) {
        throw std::runtime_error("Too many colors for palette");
    }
    auto offset = 0 ;
    auto current = setcolor.begin();
    while (current!= setcolor.end()){
        colors.at(offset) = *current ;
        offset++;
        current++;
    }
}
//=================================================================================
palette_t::palette_t(const std::vector<std::uint8_t> &buffer):palette_t(){
    if (buffer.size() < 512){
        throw std::runtime_error("Buffer to small for palette");
    }
    const std::uint16_t * color = reinterpret_cast<const std::uint16_t*>(buffer.data());
    for (auto j= 0 ; j< 256;j++){
        colors.at(j) = ((*(color+j)) &0x7fff);
    }
}

//=================================================================================
auto palette_t::operator[](size_t index) const ->const std::uint16_t& {
    return colors.at(index) ;
}
//=================================================================================
auto palette_t::operator[](size_t index)  -> std::uint16_t& {
    return colors.at(index) ;
}
//=================================================================================
auto palette_t::indexFor(std::uint16_t color) const ->std::uint8_t {
    auto iter = std::find_if(colors.begin(),colors.end(),[color](const std::uint16_t &value){
        return color == value ;
    });
    if (iter != colors.end()){
        return static_cast<std::uint8_t>(std::distance(colors.begin(), iter));
    }
    throw std::runtime_error("Color was not in palette.");
}
//=================================================================================
auto palette_t::data() const ->std::vector<std::uint8_t> {
    auto rvalue = std::vector<std::uint8_t>() ;
    for (const auto color: colors){
        rvalue.push_back(static_cast<std::uint8_t>((color&0xFF)));
        rvalue.push_back(static_cast<std::uint8_t>(((color>>8)&0xFF)));
    }
    return rvalue ;
}

//=================================================================================
// anim_frame
//=================================================================================

const std::string anim_frame::header="has_image,center_x,center_y";

//=================================================================================
auto anim_frame::valuesInImage() const ->std::map<int, std::vector<std::pair<int,int>>> {
    auto start = -1;
    auto end = -1 ;
    auto [width,height] = image.size() ;
    auto rvalue = std::map<int,std::vector<std::pair<int,int>>>() ;
    for (auto y = 0 ; y < height; y++){
        start = -1 ;
        end = -1 ;
        for (auto x = 0 ; x < width; x++){
            if (image.pixel(x, y) != 0){
                // There is a color
                if ( start == -1) {
                    start = x ;
                    end = x ;
                }
                else {
                    end = x ;
                }
            }
            else {
                if (start != -1){
                    rvalue[y].push_back(std::make_pair(start, end));
                    start = -1;
                    end = -1 ;
                }
            }
            if (x == (width-1) ){
                if (start !=-1){
                    rvalue[y].push_back(std::make_pair(start, end));
                    start = -1 ;
                    end = -1 ;
                }
            }
            
        }
    }
    return rvalue ;
}
//=================================================================================
anim_frame::anim_frame(size_t size , size_t offset, const std::vector<std::uint8_t> &buffer, const palette_t &palette) {
    if (size < 8){
        throw std::runtime_error("Invalid size for animation frame data.");
    }
    auto  data = buffer.data() +offset ;
    auto width = std::int32_t(0) ;
    auto height = std::int32_t(0);
    offset = 0 ;
    std::copy(data+offset,data+offset+2,reinterpret_cast<std::uint8_t*>(&centerx));
    offset+=2 ;
    std::copy(data+offset,data+offset+2,reinterpret_cast<std::uint8_t*>(&centery));
    offset+=2 ;
    std::copy(data+offset,data+offset+2,reinterpret_cast<std::uint8_t*>(&width));
    offset+=2 ;
    std::copy(data+offset,data+offset+2,reinterpret_cast<std::uint8_t*>(&height));
    offset+=2 ;
    image = bitmap_t<std::uint16_t>(width,height) ;
    if (!image.empty()) {
        auto xbase = centerx - 0x200 ;
        auto ybase = (centery+height) - 0x200 ;
        auto y = ybase;
        auto x = xbase ;
        auto header = std::uint32_t(0) ;
        std::copy(data+offset, data+offset+4, reinterpret_cast<std::uint8_t*>(&header));
        offset+=4 ;
        while (header != 0x7FFF7FFF){
            header ^= doubleXor ;
            auto deltay = ((header>>12) & 0x3ff) ;
            auto deltax = (header>>22) &0x3ff ;
            auto runlength = header&0xFFF ;
            for (std::uint32_t j=0 ;j<runlength;j++){
                auto color = palette[(*(data+offset))];
                offset++;
                image.pixel(x+deltax+j,y+deltay) = color ;
            }
            std::copy(data+offset, data+offset+4, reinterpret_cast<std::uint8_t*>(&header));
            offset+=4 ;
        }
    }
}

//=================================================================================
anim_frame::anim_frame(size_t frame, const std::string &line, const std::filesystem::path &path):anim_frame() {
    auto values = strutil::parse(line,",") ;
    auto [id,rest] = strutil::split(path.stem().string(),".");
    
    switch(values.size()){
        default:
        case 3:{
            centery = static_cast<std::int16_t>(std::stoi(values.at(2),nullptr,0)) ;
            [[fallthrough]];
        }
        case 2:{
            centerx = static_cast<std::int16_t>(std::stoi(values.at(1),nullptr,0)) ;
            [[fallthrough]];
        }
        case 1:{
            auto has_image = static_cast<bool>(std::stoi(values.at(0),nullptr,0)) ;
            if (has_image){
                auto ifile = id +"."s+std::to_string(frame)+(rest.empty()?""s:"-"s+rest)+".bmp" ;
                auto ipath = path ;
                ipath.replace_filename(std::filesystem::path(ifile));
                auto input = std::ifstream(ipath.string(),std::ios::binary);
                if (!input.is_open()){
                    throw std::runtime_error("Unable to open: "s+ipath.string());
                }
                image = bitmap_t<std::uint16_t>::fromBMP(input);
            }
            [[fallthrough]];
        }
        case 0:
            break;
    }
}

//=================================================================================
auto anim_frame::description() const ->std::string {
    auto output = std::stringstream() ;
    output <<(image.empty()?0:1)<<","<<centerx<<","<<centery;
    return output.str();
}
//=================================================================================
auto anim_frame::data(const palette_t &palette ) const ->std::vector<std::uint8_t> {
    if (image.empty()){
        return std::vector<std::uint8_t>();
    }
    auto nonblanks = valuesInImage() ;
    auto rvalue = std::vector<std::uint8_t>(8,0) ;
    auto offset = size_t(0) ;
    auto [width,height] = image.size();
    std::copy(reinterpret_cast<const std::uint8_t*>(&centerx),reinterpret_cast<const std::uint8_t*>(&centerx)+2,rvalue.begin()+offset);
    offset+=2 ;
    std::copy(reinterpret_cast<const std::uint8_t*>(&centery),reinterpret_cast<const std::uint8_t*>(&centery)+2,rvalue.begin()+offset);
    offset+=2 ;
    std::copy(reinterpret_cast<const std::uint8_t*>(&width),reinterpret_cast<const std::uint8_t*>(&width)+2,rvalue.begin()+offset);
    offset+=2 ;
    std::copy(reinterpret_cast<const std::uint8_t*>(&height),reinterpret_cast<const std::uint8_t*>(&height)+2,rvalue.begin()+offset);
    offset+=2 ;
    // Now we get to add the data for the image
    auto xbase = centerx - 0x200 ;
    auto ybase = (centery+height) - 0x200 ;
    for (const auto &[y,ranges]:nonblanks){
        for (const auto &[start,end]:ranges){
            //  First specify the offset and run length
            auto deltay = static_cast<std::uint32_t>((y-ybase)& 0xFFF) << 12 ;
            auto deltax = static_cast<std::uint32_t>((start -xbase) &0xFFF) <<22 ;
            auto runlength = std::uint32_t(end-start +1) &0xFFF;
            auto header = deltay | deltax | runlength ;
            header ^= doubleXor ;
            
            
            rvalue.resize(rvalue.size()+4,0) ;
            std::copy(reinterpret_cast<const std::uint8_t*>(&header), reinterpret_cast<const std::uint8_t*>(&header)+4,rvalue.begin()+offset);
            offset+=4 ;
            
            // Now do the color index
            for (auto j= start; j<= end; j++){
                auto colval = image.pixel(j,y) ;
                rvalue.push_back(palette.indexFor(colval));
                offset+=1 ;
            }
        }
    }
    // And we finished with saying we are complete!
    rvalue.push_back(0xFF) ;
    rvalue.push_back(0x7F) ;
    rvalue.push_back(0xFF) ;
    rvalue.push_back(0x7F) ;
    
    
    return rvalue ;
}
//=================================================================================
// animation_t
//=================================================================================


//=============================================================================================
auto animation_t::framesFromAnimData(const std::vector<std::uint8_t> &buffer) ->std::vector<anim_frame>{
    auto rvalue = std::vector<anim_frame>() ;
    this->palette = palette_t(buffer) ;
    auto offset = size_t(512) ;  // Skip past the palette
    auto frame_count = *reinterpret_cast<const std::uint32_t*>((buffer.data()+offset));
    offset +=4 ;
    // Offsets are bytes from the end of the palette ;
    auto offsets = std::vector<std::uint32_t>(frame_count,0);
    for (std::uint32_t count = 0 ; count < frame_count;count++){
        offsets.at(count) = *reinterpret_cast<const std::uint32_t*>((buffer.data()+offset));
        offset+=4 ;
    }
    for (size_t frame = 0 ; frame < offsets.size();frame++){
        auto entry = anim_frame() ;
        if (offsets.at(frame)!= 0){
            auto dataoffset = static_cast<size_t>(512 + offsets.at(frame)) ;
            auto size = buffer.size() - dataoffset ;
            if (frame < offsets.size()-1) {
                size = offsets.at(frame+1) - offsets.at(frame) ;
            }
            entry = anim_frame(size, dataoffset, buffer, this->palette);
        }
        rvalue.push_back(entry);
        
    }
    return rvalue ;
}
//==================================================================================
animation_t::animation_t(const std::vector<std::uint8_t> &buffer) {
    frames= framesFromAnimData(buffer);
}
//==================================================================================
animation_t::animation_t(std::ifstream &input,const std::filesystem::path &path){
    auto buffer = std::vector<char>(4096,0) ;
    auto frame = size_t(0) ;
    while (input.good() && !input.eof()){
        input.getline(buffer.data(),4095);
        if (input.gcount()>0){
            std::string line = buffer.data();
            line = strutil::trim(strutil::strip(line,"//"));
            if (!line.empty()){
                if ((line.at(0) >=48 ) && (line.at(0) <=57)){
                    frames.push_back(anim_frame(frame,line, path));
                    frame++;
                }
            }
        }
    }
    palette = palette_t(frames);
}
//==================================================================================
auto animation_t::description() const ->std::string {
    auto output = std::stringstream();
    output << anim_frame::header<<"\n";
    for (auto &frame:frames) {
        output << frame.description()<<"\n";
    }
    return output.str();
}
//==================================================================================
auto animation_t::data() const ->std::vector<std::uint8_t> {
    auto rvalue = std::vector<std::uint8_t>() ;
    
    // palette should have all ready been made
    auto palette_data = palette.data() ;
    rvalue.insert(rvalue.end(),palette_data.begin(),palette_data.end());
    auto framecount = std::uint32_t(frames.size());
    auto size = size_t(4 + (4*framecount));
    rvalue.resize(rvalue.size()+size);
    auto end_palette = size_t(palette_data.size()) ;
    auto offsets = size_t(palette_data.size()+4) ;
    auto dataoffset = offsets+(framecount*4);
    std::copy(reinterpret_cast<const std::uint8_t*>(&framecount),reinterpret_cast<const std::uint8_t*>(&framecount)+4,rvalue.begin()+end_palette) ;
    
    for (size_t j= 0 ;j<frames.size();j++){
        
        auto data = frames.at(j).data(palette) ;
        if (!data.empty()){
            auto location = dataoffset - end_palette ;
            std::copy(reinterpret_cast<std::uint8_t*>(&location),reinterpret_cast<std::uint8_t*>(&location)+4,rvalue.begin()+offsets );
            offsets +=4 ;
            rvalue.insert(rvalue.end(),data.begin(),data.end());
            dataoffset += data.size() ;
        }
        else {
            auto location = 0 ;
            std::copy(reinterpret_cast<std::uint8_t*>(&location),reinterpret_cast<std::uint8_t*>(&location)+4,rvalue.begin()+offsets );
            offsets+=4 ;
        }
        
    }
    return rvalue ;
}
//==================================================================================
auto animation_t::size() const ->size_t {
    return frames.size();
}
//==================================================================================
auto animation_t::operator[](size_t index) const ->const anim_frame& {
    return frames.at(index) ;
}
//==================================================================================
auto animation_t::operator[](size_t index) ->anim_frame& {
    return frames.at(index);
}
