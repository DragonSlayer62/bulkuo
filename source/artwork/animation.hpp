//

#ifndef animation_hpp
#define animation_hpp

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <utility>

#include "bitmap.hpp"

class anim_frame;
//=================================================================================
//===========================================================================================
class palette_t{
public:
    std::vector<std::uint16_t> colors ;
    palette_t() ;
    palette_t(const std::vector<bitmap_t<std::uint16_t>> &images) ;
    palette_t(const std::vector<std::uint8_t> &buffer);
    palette_t(const std::vector<anim_frame> &frames);
    auto operator[](size_t index) const ->const std::uint16_t&;
    auto operator[](size_t index)  -> std::uint16_t&;
    auto indexFor(std::uint16_t color) const ->std::uint8_t ;
    auto data() const ->std::vector<std::uint8_t> ;
};


//=============================================================================================
class anim_frame{
private:
    static constexpr auto doubleXor = std::uint32_t((0x200 << 22) | (0x200 << 12)) ;
    auto valuesInImage() const ->std::map<int, std::vector<std::pair<int,int>>> ;
public:
    static const std::string header ;
    std::int16_t centerx ;
    std::int16_t centery ;
    bitmap_t<std::uint16_t> image ;
    anim_frame() :centerx(0),centery(0){}
    anim_frame(size_t size , size_t offset, const std::vector<std::uint8_t> &buffer, const palette_t &palette);
    anim_frame(size_t frame,const std::string &line, const std::filesystem::path &path) ;
    auto description() const ->std::string ;
    auto data(const palette_t &palette) const ->std::vector<std::uint8_t> ;
};

//=============================================================================================
class animation_t {
    palette_t palette ;
    auto framesFromAnimData(const std::vector<std::uint8_t> &buffer) ->std::vector<anim_frame>;
public:
    std::vector<anim_frame> frames ;
    animation_t(const std::vector<std::uint8_t> &buffer);
    animation_t()=default ;
    animation_t(std::ifstream &input,const std::filesystem::path &path);
    auto description() const ->std::string ;
    auto data() const ->std::vector<std::uint8_t> ;
    auto size() const ->size_t ;
    auto save(size_t id, std::filesystem::path &path) ->void ;
    auto operator[](size_t index) const ->const anim_frame& ;
    auto operator[](size_t index) ->anim_frame& ;
};

#endif /* animation_hpp */
