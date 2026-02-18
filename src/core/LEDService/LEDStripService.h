//
// Created by rmaks on 15-Feb-26.
//

#ifndef AMBILIGHTCONTROLLER_LEDSTRIPSERVICE_H
#define AMBILIGHTCONTROLLER_LEDSTRIPSERVICE_H



#include "core/Global/Global.hpp"

#include <vector>


#define DATA_PIN 5
#define COLOR_ORDER GRB


//todo
static std::vector<const char *> direction_TRBL{"top", "right", "bottom", "left"};
static std::vector<const char *> direction_RBLT{"right", "bottom", "left", "top"};
static std::vector<const char *> direction_BTLR{"bottom", "left", "top", "right"};
static std::vector<const char *> direction_LTRB{"left", "top", "right", "bottom"};
//todo
static const std::map<RGBDirection, std::vector<const char *>> RgbFlowDirections{
        {TRBL, direction_TRBL},
        {RBLT, direction_RBLT},
        {BTLR, direction_BTLR},
        {LTRB, direction_LTRB}
};


class LEDStripService {

public:
    void init(unsigned int ledCount);

    void testLEDStrip();
    void fadeOut();
    void setColors(LEDState state);
    void setBrightness(unsigned int brightnessValue);
    void initColorState(LEDState state);
    void initColorState(HTMLColorCode plainColor);
    void fillWhite();
    void fillColor(HTMLColorCode colorCode);
    void clear(bool clearState = false);
    void restoreLastState();
    unsigned int getBrightness();

    LEDState currentState() const;
private:
    LEDState state;
    CRGB *LEDStrip = nullptr;
    unsigned int ledCount = 0;
};


#endif //AMBILIGHTCONTROLLER_LEDSTRIPSERVICE_H
