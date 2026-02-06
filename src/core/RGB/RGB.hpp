//
// Created by rmaks on 05-Feb-26.
//

#ifndef AMBILIGHTCONTROLLER_RGB_HPP
#define AMBILIGHTCONTROLLER_RGB_HPP

#include "core/Global/Global.hpp"

#include <vector>


std::vector<const char *> direction_TRBL{"top", "right", "bottom", "left"};
std::vector<const char *> direction_RBLT{"right", "bottom", "left", "top"};
std::vector<const char *> direction_BTLR{"bottom", "left", "top", "right"};
std::vector<const char *> direction_LTRB{"left", "top", "right", "bottom"};

const std::map<RGBDirection, std::vector<const char *>> RgbFlowDirections {
        {TRBL, direction_TRBL},
        {RBLT, direction_RBLT},
        {BTLR, direction_BTLR},
        {LTRB, direction_LTRB}
};

#endif //AMBILIGHTCONTROLLER_RGB_HPP
