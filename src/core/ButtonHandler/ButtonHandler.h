//
// Created by rmaks on 21-Mar-26.
//

#ifndef AMBILIGHTCONTROLLER_BUTTONHANDLER_H
#define AMBILIGHTCONTROLLER_BUTTONHANDLER_H

#pragma once
#include <Arduino.h>
#include <functional>

typedef std::function<void()> ButtonCallback;

class ButtonHandler {
public:
    ButtonHandler(int buttonPin, bool activeHigh = false);

    void begin();
    void handle();

    void attachShortPressHandler(ButtonCallback cb);
    void attachLongPressHandler(ButtonCallback cb);
    void attachDoublePressHandler(ButtonCallback cb);

private:
    int _buttonPin;
    bool _activeHigh;

    bool _activeState;
    bool _inactiveState;

    void initPin();

    ButtonCallback _shortPressCb = nullptr;
    ButtonCallback _longPressCb = nullptr;
    ButtonCallback _doublePressCb = nullptr;

    bool _lastSteadyState;
    bool _lastFlickerableState;
    unsigned long _lastDebounceTime = 0;
    const unsigned long _debounceDelay = 50;

    unsigned long _pressedTime = 0;
    unsigned long _lastClickTime = 0;

    bool _isPressing = false;
    bool _isLongDetected = false;
    int _clickCount = 0;

    const unsigned long _longPressDuration = 600;
    const unsigned long _doubleClickGap = 250;
};

#endif //AMBILIGHTCONTROLLER_BUTTONHANDLER_H
