//
// Created by rmaks on 06-Apr-24.
//

#ifndef TEMPLATE_ESP8266_CORE_SETTINGS_H
#define TEMPLATE_ESP8266_CORE_SETTINGS_H

#include <Arduino.h>
#include "core/Global/Global.hpp"
#include "ArduinoJson.h"
typedef StaticJsonDocument<sizeof (EEPROMSettings)> JsonSettings;
typedef enum {DATA_SAVED, FAILED, NO_DATA} DataSaveResult;

class Settings {

public:
    void loadSettings();
    void loadAdditionalData();

    DataSaveResult saveSettings(String settings);
    DataSaveResult saveSettings(DynamicJsonDocument &json);


    WiFiSettings wifiSettings();
    EEPROMSettings eepromSettings() const;

    JsonSettings eepromSettingsObj() const;

    char *customHotspotSsid();

    bool loggerEnabled() const;
    bool clientWebAccessEnabled() const;
    bool otaUpdateOnClientMode() const;
    bool saveLastState() const;
    unsigned int ledCount() const;
    RGBDirection ledFlowDirection() const;
    LoggerLevel loggerLevel() const;
    CONTROLLER_WIFI_MODE wiFiMode() const;

    void handleVersionChange(unsigned int currentFWVersion, bool requireEEPROMFormat);

private:
    EEPROMSettings _eepromSettings;
    String _dataFromFS;

    DataSaveResult processSettingsDataToSave(DynamicJsonDocument &json);
    void writeButtonEepromSettings(JsonVariant const & jsonSettings);
    void saveAdditionalData(String data);
    void writeToEEPROM(EEPROMSettings settings);
    void clearEeprom();

    String dataFromFS(const String& fileName);
};


#endif //TEMPLATE_ESP8266_CORE_SETTINGS_H
