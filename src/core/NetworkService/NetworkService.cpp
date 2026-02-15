#include "NetworkService.h"
#include <ESP8266WiFi.h>
#include <Arduino.h>

void NetworkService::connectToWiFi(const String &ssid, const String &pass) {
    auto result = this->initWiFiConnection(ssid, pass);
    if (result == CONNECTED) {
        logger.log("[Network] IP: ", WiFi.localIP().toString(), " Hostname: ", WiFi.hostname().c_str());
    } else {
        logger.log("[Network] Failed to init network connection");
    }
}

void NetworkService::connectToWiFi(const WiFiSettings wiFiConnDetails) {
    auto ssid = wiFiConnDetails.ssid;
    auto pass = wiFiConnDetails.password;
    auto result = this->initWiFiConnection(ssid, pass);
    if (result == CONNECTED) {
        logger.log("[Network] IP: ", WiFi.localIP().toString(), " Hostname: ", WiFi.hostname().c_str());
    } else {
        logger.log("[Network] Failed to init network connection");
    }
}

void NetworkService::buttonHotspot(bool isOn, const char* ssid, const char* pass) {
    if (isOn) {
        WiFi.mode(WIFI_STA);
        WiFi.softAP(ssid, pass);
        IPAddress IP = WiFi.softAPIP();
        logger.log("[Network] Set AP: ", ssid, " IP address: ", IP.toString());
    } else {
        WiFi.softAPdisconnect(isOn);
        logger.log("[Network] AP is off.");
    }
}

Networks NetworkService::wiFiList() {
    Networks list;
    list.size = WiFi.scanNetworks();
    list.arr = new String[list.size];

    logger.log("[Network] Networks found: ", list.size);

    for (int i = 0 ; i < list.size; i++) {
        logger.logSerial("[Network] SSID: ", WiFi.SSID(i).c_str());
        list.arr[i] = WiFi.SSID(i);
    }

    return list;
}

bool NetworkService::isConnectedToWiFi() {
    return WiFi.isConnected();
}

bool NetworkService::isAPMode() {
    return (WiFi.getMode() == WIFI_AP) || (WiFi.getMode() == WIFI_AP_STA);
}

WiFiConnectionResult NetworkService::initWiFiConnection(const String &ssid, const String &pass, unsigned int retryAttempt) {
    this->initWirelessModule();
    wifi_station_set_hostname(DEVICE_HOSTNAME);
    WiFi.setAutoConnect(false);
    WiFi.begin(ssid, pass);

    if (retryAttempt) {
        for (unsigned int i = 0 ; i < retryAttempt; i++) {
            logger.logSerial("[Network] Connecting to ", ssid, " / ", pass, " try: ", i, "/", retryAttempt);
            delay(1000);
            if (WiFi.status() == WL_CONNECTED) {
                return CONNECTED;
            }
        }
        return CONNECTION_FAILED;
    }
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        logger.logSerial("[Network] Connecting to ", ssid, " / ", pass);
    }
    return CONNECTED;
}

void NetworkService::setWiFiMode(CONTROLLER_WIFI_MODE mode) {
    this->_mode = mode;
}

String NetworkService::getWiFIMode() const {
    return this->_modeMap.at(WiFi.getPhyMode());
}

void NetworkService::initWirelessModule() {
    WiFi.mode(WIFI_STA);
    if (this->_mode) {
        logger.log("[Network] Set WiFi mode to: ", this->_modeMap.at(static_cast<const WiFiPhyMode>(this->_mode)));
        WiFi.setPhyMode(static_cast<WiFiPhyMode_t>(this->_mode));
    }
}
