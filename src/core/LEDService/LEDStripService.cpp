//
// Created by rmaks on 15-Feb-26.
//

#include "LEDStripService.h"


void LEDStripService::init(unsigned int ledCount) {
    logger.log("[LEDStripService] ", "Init LED Strip. LEDs count: ", ledCount);
    logger.log("[LEDStripService] ", "Init LED Strip. Data pin: ", DATA_PIN);
    this->ledCount = ledCount;
    if(LEDStrip != nullptr) {
        logger.logSerial("[LEDStripService] ", "Service already exist. Recreating...");
        delete LEDStrip;
    }
    LEDStrip = new CRGB[ledCount];
    FastLED.addLeds<WS2811, DATA_PIN, COLOR_ORDER>(LEDStrip, ledCount);
    FastLED.clear();
    FastLED.show();
    logger.logSerial("[LEDStripService] ", "Init LED Strip. Done");
}

void LEDStripService::clear(bool clearState) {
    logger.log("[LEDStripService] ", "Clear colors. Clear state: ", clearState);
    FastLED.clear();
    FastLED.show();
    if(clearState) this->state.clear();
}

void LEDStripService::testLEDStrip() {
    logger.log("[LEDStripService] ", "Test LED Strip. LEDs count: ", this->ledCount);
    for (int whiteLed = 0; whiteLed < this->ledCount; whiteLed = whiteLed + 1) {
        LEDStrip[whiteLed] = CRGB::White;
        FastLED.show();
        delay(50);
        LEDStrip[whiteLed] = CRGB::Black;
    }
}

void LEDStripService::fillColor(HTMLColorCode colorCode) {
    logger.log("[LEDStripService] ", "Fill with color by code: ", colorCode);
    for (int whiteLed = 0; whiteLed < this->ledCount; whiteLed = whiteLed + 1) {
        LEDStrip[whiteLed] = colorCode;
    }
    FastLED.show();
}

void LEDStripService::fillWhite() {
    logger.log("[LEDStripService] ", "Fill white color");
    for (int whiteLed = 0; whiteLed < this->ledCount; whiteLed = whiteLed + 1) {
        LEDStrip[whiteLed] = CRGB::White;
    }
    FastLED.show();
}

void LEDStripService::fadeOut() {
    logger.log("[LEDStripService] ", "Fade Out");
    for (int i = 0; i < 40; i++) {
        fadeToBlackBy(LEDStrip, ledCount, 15);
        FastLED.show();
        delay(30);
    }
    FastLED.clear();
    FastLED.show();
}

void LEDStripService::setColors(LEDState state) {
    logger.log("[LEDStripService] ", "Set Strip Colors");
    this->state = state;
    for(size_t i = 0; i < this->state.size(); i++) {
        LEDStrip[i] = this->state[i];
    }
    FastLED.show();
}

void LEDStripService::restoreLastState() {
    logger.log("[LEDStripService] ", "Restore last state");
    for(size_t i = 0; i < this->state.size(); i++) {
        LEDStrip[i] = this->state[i];
    }
    FastLED.show();
}

void LEDStripService::initColorState(LEDState state) {
    this->state = state;
}

void LEDStripService::initColorState(HTMLColorCode plainColor) {
    this->state.assign(this->ledCount, plainColor);
}

LEDState LEDStripService::currentState() const {
    return this->state;
}

void LEDStripService::setBrightness(unsigned int brightnessValue) {
    logger.log("[LEDStripService] ", "Set Brightness Value to: ", brightnessValue);
    FastLED.setBrightness(brightnessValue);
    FastLED.show();
}

unsigned int LEDStripService::getBrightness() {
    return FastLED.getBrightness();
}
