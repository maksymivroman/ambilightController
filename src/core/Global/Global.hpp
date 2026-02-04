//
// Created by rmaks on 06-Apr-24.
//

#ifndef TEMPLATE_ESP8266_CORE_GLOBAL_H
#define TEMPLATE_ESP8266_CORE_GLOBAL_H



#include <Arduino.h>
#include "core/Logger/Logger.h"

extern Logger logger;

static const char DEVICE_HOSTNAME[] = "One core device";
static const char DEVICE_HOTSPOT_PASS[] = "12345678";
static const char ESP_DEFAULT_SSID[] = "one core-";
static const char FS_ADDITIONAL_DATA_PATH[] = "/data.json";

struct WiFiSettings {
    String ssid;
    String password;
};

struct Networks {
    String *arr = nullptr;
    int8_t size = 0;
};

struct EEPROMSettings {
    bool useCustomHSsid = false;
    bool clientWebAccess = false;
    bool loggerEnabled = true;
    bool enableOtaUpdate = false;
    unsigned int loggerLevel = 0;
    unsigned int fwVersion = 0;
    char wifiSsid[256]{};
    char wifiPass[256]{};
    char hotspotSsid[32]{};
};

enum StartupMode {
    RUN,
    SETUP
};

#endif //TEMPLATE_ESP8266_CORE_GLOBAL_H