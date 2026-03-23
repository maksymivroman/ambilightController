//
// Created by rmaks on 16-Feb-26.
//

#ifndef AMBILIGHTCONTROLLER_ID_H
#define AMBILIGHTCONTROLLER_ID_H


#include <Arduino.h>
#include "ArduinoJson.h"
#include <ESP8266WiFi.h>

#include "core/Global/Global.hpp"
#include "core/Settings/Settings.h"
#include "core/Version/Version.h"

extern Settings deviceSettings;
extern Version deviceVersion;


class ID {
public:

    struct Action {
        String name;
        String action;
    };

    static void addAction(const String & name, const String & action);
    static void setUrl(const String & hostUrl, const String & uiUrl);
    static void setUrl(const String & hostUrl, const String & uiUrl, const String &widgetUrl);
    static String toJsonString();

    static void addStateHandler(std::function<String()> stateHandlerFn){
        _stateHandlerFn = stateHandlerFn;
    }

private:
    static const size_t MAX_ACTIONS = 10;
    static Action _actions[MAX_ACTIONS];
    static const char* _type;
    static const char* _id;
    static String _hostUrl;
    static String _uiUrl;
    static String _widgetUrl;
    static size_t _actionCount;
    static String _state;

    static std::function<String()> _stateHandlerFn;
};


#endif //AMBILIGHTCONTROLLER_ID_H
