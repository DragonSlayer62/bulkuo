//Copyright © 2022 Charles Kerr. All rights reserved.

#include "animsupport.hpp"

#include <iostream>
#include <stdexcept>

#include "../types.hpp"
#include "actutility.hpp"

#include "../artwork/bitmap.hpp"
#include "../artwork/animation.hpp"

using namespace std::string_literals;


//=================================================================================
auto extractAnimation(const argument_t &arg, std::uint32_t id,const std::vector<std::uint8_t> &buffer )->void {
    auto image_set = decipherAnimData(buffer) ;
    auto frame = 0 ;
    for (const auto &[imageCenterX,imageCenterY,image]:image_set) {
        if (!image.empty()){
            // We need to the path for the bmp file
            
        }
    }
}
