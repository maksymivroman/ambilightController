//
// Created by rmaks on 06-Apr-24.
//


#ifndef TEMPLATE_ESP8266_CORE_GLOBAL_H
#define TEMPLATE_ESP8266_CORE_GLOBAL_H

#pragma once

#include <Arduino.h>
#include "core/Logger/Logger.h"
#include <ESP8266WiFi.h>

#include <map>
#include <vector>

extern Logger logger;

static const char DEVICE_HOSTNAME[] = "Ambilight Controller";
static const char DEVICE_HOTSPOT_PASS[] = "12345678";
static const char ESP_DEFAULT_SSID[] = "Ambilight Controller-";
static const char FS_ADDITIONAL_DATA_PATH[] = "/data.json";

enum CONTROLLER_WIFI_MODE {
    AUTO,
    MODE_11B = WiFiPhyMode::WIFI_PHY_MODE_11B,
    MODE_11G = WiFiPhyMode::WIFI_PHY_MODE_11G,
    MODE_11N = WiFiPhyMode::WIFI_PHY_MODE_11N,
};

struct WiFiSettings {
    String ssid;
    String password;
};

struct Networks {
    String *arr = nullptr;
    int8_t size = 0;
};

enum RGBDirection {
    TRBL,
    RBLT,
    BTLR,
    LTRB
};


struct EEPROMSettings {
    bool useCustomHSsid = false;
    bool clientWebAccess = false;
    bool loggerEnabled = true;
    bool enableOtaUpdate = false;
    bool saveLastState = false;
    unsigned int ledCount = 1;
    RGBDirection ledFlowDirection = TRBL;
    CONTROLLER_WIFI_MODE wiFiMode = AUTO;
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