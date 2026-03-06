#ifndef TEMPLATE_ESP8266_CORE_NETWORKSERVICE_H
#define TEMPLATE_ESP8266_CORE_NETWORKSERVICE_H

#include <Arduino.h>
#include "core/Global/Global.hpp"
#include <map>

typedef enum WIFI_CONNECTION_RESULT {CONNECTED, CONNECTION_FAILED, CONNECTION_PENDING} WiFiConnectionResult;
typedef std::map<WiFiPhyMode, String> WiFiModeMap;

class NetworkService {

public:
    void buttonHotspot(bool isOn, const char* ssid, const char* pass);
    void connectToWiFi(const String& ssid, const String& pass);
    void connectToWiFi(const WiFiSettings wiFiConnDetails);
    Networks wiFiList();
    bool isConnectedToWiFi();
    bool isAPMode();
    void setWiFiMode(CONTROLLER_WIFI_MODE mode);
    String ipAddress() const;

    String getWiFIMode() const;
private:
    void initWirelessModule();

    CONTROLLER_WIFI_MODE _mode = AUTO;
    WiFiModeMap _modeMap = {
            {WIFI_PHY_MODE_11B, "11B"},
            {WIFI_PHY_MODE_11G, "11G"},
            {WIFI_PHY_MODE_11N, "11N"}
    };
    WiFiConnectionResult initWiFiConnection(const String& ssid, const String& pass, unsigned int retryAttempt = 0);

    bool _isConnectionInitialized {false};
};

#endif