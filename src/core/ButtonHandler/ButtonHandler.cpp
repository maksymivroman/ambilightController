#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(int buttonPin, bool activeHigh)
        : _buttonPin(buttonPin), _activeHigh(activeHigh)
{
    _activeState = _activeHigh ? HIGH : LOW;
    _inactiveState = _activeHigh ? LOW : HIGH;

    _lastSteadyState = _inactiveState;
    _lastFlickerableState = _inactiveState;
}

void ButtonHandler::begin() {
    initPin();
}

void ButtonHandler::initPin() {
    if (_activeHigh) {
        pinMode(_buttonPin, INPUT);
    } else {
        pinMode(_buttonPin, INPUT_PULLUP);
    }
}

void ButtonHandler::attachShortPressHandler(ButtonCallback cb) { _shortPressCb = cb; }
void ButtonHandler::attachLongPressHandler(ButtonCallback cb) { _longPressCb = cb; }
void ButtonHandler::attachDoublePressHandler(ButtonCallback cb) { _doublePressCb = cb; }

void ButtonHandler::handle() {
    bool currentState = digitalRead(_buttonPin);

    if (currentState != _lastFlickerableState) {
        _lastDebounceTime = millis();
        _lastFlickerableState = currentState;
    }

    if ((millis() - _lastDebounceTime) > _debounceDelay) {
        if (currentState != _lastSteadyState) {
            _lastSteadyState = currentState;


            if (_lastSteadyState == _activeState) {
                _pressedTime = millis();
                _isPressing = true;
                _isLongDetected = false;
            }

            else if (_lastSteadyState == _inactiveState) {
                _isPressing = false;

                if (!_isLongDetected) {
                    _clickCount++;
                    _lastClickTime = millis();
                }
            }
        }
    }

    if (_isPressing && !_isLongDetected) {
        if ((millis() - _pressedTime) > _longPressDuration) {
            _isLongDetected = true;
            _clickCount = 0;

            if (_longPressCb) {
                _longPressCb();
            }
        }
    }

    if (!_isPressing && _clickCount > 0) {
        if ((millis() - _lastClickTime) > _doubleClickGap) {

            if (_clickCount == 1 && _shortPressCb) {
                _shortPressCb();
            } else if (_clickCount == 2 && _doublePressCb) {
                _doublePressCb();
            }

            _clickCount = 0;
        }
    }
}
