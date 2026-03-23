//
// Created by rmaks on 15-Feb-26.
//

#include "LEDStripService.h"


void LEDStripService::init(unsigned int ledCount) {
    if(ledCount > 1000) {
        logger.log("[LEDStripService] ", "Init LED Strip. ERROR: cannot set LED count to ", ledCount);
        ledCount = 1;
    }
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
    this->setLEDStripMode(LED_OFF);
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
    this->setLEDStripMode(LED_ON);

}

LEDState LEDStripService::fillWhite() {
    logger.log("[LEDStripService] ", "Fill white color");
    auto whiteColor = CRGB::White;
    LEDState stateColors;
    stateColors.clear();
    stateColors.reserve(this->ledCount);
    std::fill(stateColors.begin(), stateColors.end(), whiteColor);

    for (int whiteLed = 0; whiteLed < this->ledCount; whiteLed = whiteLed + 1) {
        LEDStrip[whiteLed] = whiteColor;
    }
    this->setLEDStripMode(LED_ON);
    return stateColors;
}

void LEDStripService::fadeOut() {
    logger.log("[LEDStripService] ", "Fade Out");
    for (int i = 0; i < 40; i++) {
        fadeToBlackBy(LEDStrip, ledCount, 15);
        FastLED.show();
        delay(30);
    }
    this->setLEDStripMode(LED_OFF);
}

void LEDStripService::setColors(LEDState state) {
    logger.log("[LEDStripService] ", "Set Strip Colors");
    this->state = state;
    for(size_t i = 0; i < this->state.size(); i++) {
        LEDStrip[i] = this->state[i];
    }
    this->setAnimationRanges();
    this->setLEDStripMode(LED_ON);
}

void LEDStripService::restoreLastState() {
    logger.log("[LEDStripService] ", "Restore last state");
    for(size_t i = 0; i < this->state.size(); i++) {
        LEDStrip[i] = this->state[i];
    }
    this->setLEDStripMode(LED_ON);
}

void LEDStripService::initColorState(LEDState state) {
    this->state = state;
    this->setAnimationRanges();
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

void LEDStripService::animate(boolean byStateRange) {
    if(byStateRange && !this->state.empty()) {
        this->renderColorFlow(this->firstHSV.hue, this->lastHSV.hue);
    } else {
        this->renderColorFlow(10,102);
    }
}

void LEDStripService::renderColorFlow(byte rangeFrom, byte rangeTo) {
    uint8_t deltaPhase = std::max(1, 255 / static_cast<uint8_t>(this->ledCount));
    // (Якщо хочете, щоб хвиль було кілька, помножте це значення на 2 або 3)

    // 3. Проходимось по всіх пікселях
    for (int i = 0; i < this->ledCount; i++) {

        // Розраховуємо фазу для конкретного пікселя:
        // Зміщення в часі + зміщення в просторі (позиція пікселя)
        uint8_t pixelPhase = timePhase + (i * deltaPhase);

        // cubicwave8 перетворює лінійне зростання фази (0-255) у плавну хвилю (0->255->0)
        uint8_t waveVal = cubicwave8(pixelPhase);

        // Масштабуємо значення хвилі (0-255) у наш бажаний діапазон кольорів (96-192)
        uint8_t currentHue = map(waveVal, 0, 255, rangeFrom, rangeTo);

        // Застосовуємо колір (100% насиченість, 100% яскравість)
        LEDStrip[i] = CHSV(currentHue, 255, 255);
    }

    // Відправляємо дані на стрічку
    this->setLEDStripMode(LED_ANIMATION);

    // 4. Анімація: зсуваємо фазу кожні N мілісекунд (не блокуючи процесор!)
    EVERY_N_MILLISECONDS(20) {
        timePhase += 2; // Швидкість переливання. Більше значення = швидший рух.
    }
}

void LEDStripService::setAnimationRanges() {
    this->firstHSV = rgb2hsv_approximate(this->state.front());
    this->lastHSV = rgb2hsv_approximate(this->state.back());
    logger.log("[LEDStripService] Set range for animation [", firstHSV.hue, ", ", lastHSV.hue, "]");
}

LEDStripMode LEDStripService::getLEDStripMode() const {
    return this->_currentLEDStripMode;
}

void LEDStripService::setLEDStripMode(LEDStripMode mode) {
    this->_currentLEDStripMode = mode;
    switch (mode) {
        case LED_OFF:
            FastLED.clear();
            FastLED.show();
            break;
        case LED_ON:
        case LED_ANIMATION:
            FastLED.show();
            break;
    }
}
