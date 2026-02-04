#ifndef TEMPLATE_ESP8266_CORE_NETWORKSERVICE_H
#define TEMPLATE_ESP8266_CORE_NETWORKSERVICE_H

#include <Arduino.h>
#include "core/Global/Global.hpp"

typedef enum WIFI_CONNECTION_RESULT {CONNECTED, CONNECTION_FAILED} WiFiConnectionResult;

class NetworkService {

public:
    void buttonHotspot(bool isOn, const char* ssid, const char* pass);
    void connectToWiFi(const String& ssid, const String& pass);
    void connectToWiFi(const WiFiSettings wiFiConnDetails);
    Networks wiFiList();
    bool isConnectedToWiFi();
    bool isAPMode();

private:
    WiFiConnectionResult initWiFiConnection(const String& ssid, const String& pass, unsigned int retryAttempt = 0);
};

#endif