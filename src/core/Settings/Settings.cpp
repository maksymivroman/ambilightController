//
// Created by rmaks on 06-Apr-24.
//
#include <Arduino.h>


#include "Settings.h"
#include "ESPAsyncWebServer.h"

#include "ArduinoJson.h"
#include <EEPROM.h>

void Settings::loadSettings() {
    EEPROM.begin(1024);
    EEPROM.get(0, this->_eepromSettings);
    EEPROM.end();
}

WiFiSettings Settings::wifiSettings() {
    WiFiSettings _settings{
            this->_eepromSettings.wifiSsid,
            this->_eepromSettings.wifiPass
    };
    return _settings;
}

DataSaveResult Settings::saveSettings(String settings) {
    DynamicJsonDocument jsonDoc(4096);
    deserializeJson(jsonDoc, settings);
    return this->processSettingsDataToSave(jsonDoc);
}

DataSaveResult Settings::saveSettings(DynamicJsonDocument &json) {
    auto result = this->processSettingsDataToSave(json);
    if (result == DATA_SAVED) {
        logger.log("[Settings] Reload settings");
        this->loadSettings();
        this->loadAdditionalData();
    }
    return result;
}

DataSaveResult Settings::processSettingsDataToSave(DynamicJsonDocument &json) {
    const JsonVariant &configuration = json["configuration"];
    const String additionalData = json["data"];
    logger.logSerial("[Settings] additionalData: ", additionalData);
    logger.logSerial("[Settings] configuration: ", static_cast<String>(configuration));
    if (configuration.isNull()) {
        logger.log("[Settings] Failed to save: no configuration data");
        return NO_DATA;
    }
    this->writeButtonEepromSettings(configuration);
    this->saveAdditionalData(additionalData);
    return DATA_SAVED;
}

void Settings::writeButtonEepromSettings(JsonVariant const &jsonSettings) {
    auto settings = *new EEPROMSettings;

    String wiFiName = jsonSettings["wifiSsid"] | "";
    String wiFiPassword = jsonSettings["wifiPass"] | "";
    String hotspotSsid = jsonSettings["hotspotSsid"] | "";

    char ssid[256], pass[256], hSsid[32];

    wiFiName.toCharArray(ssid, 256);
    wiFiPassword.toCharArray(pass, 256);
    hotspotSsid.toCharArray(hSsid, 32);

    strcpy(settings.wifiSsid, ssid);
    strcpy(settings.wifiPass, pass);

    strcpy(settings.hotspotSsid, hSsid);

    settings.clientWebAccess = jsonSettings["clientWebAccess"].as<bool>() | false;
    settings.enableOtaUpdate = jsonSettings["enableOtaUpdate"].as<bool>() | false;
    settings.loggerEnabled = jsonSettings["loggerEnabled"].as<bool>() | false;
    settings.useCustomHSsid = jsonSettings["useHotspotSsid"].as<bool>() | false;
    settings.loggerLevel = jsonSettings["loggerLevel"].as<unsigned int>() | 0;

    settings.fwVersion = this->_eepromSettings.fwVersion;

    logger.log("[Settings][EEPROM] EEPROM config size: ", sizeof settings);

    this->writeToEEPROM(settings);
}

void Settings::saveAdditionalData(String data) {

}

void Settings::writeToEEPROM(EEPROMSettings settings) {
    logger.log("[Settings][EEPROM] Write to EEPROM ", sizeof settings, " bytes...");
    EEPROM.begin(1024);
    EEPROM.put(0, settings);
    EEPROM.end();
    logger.log("[Settings][EEPROM] Write to EEPROM. DONE");
}

bool Settings::clientWebAccessEnabled() const {
    return _eepromSettings.clientWebAccess | false;
}

bool Settings::otaUpdateOnClientMode() const {
    return _eepromSettings.enableOtaUpdate | false;
}

bool Settings::loggerEnabled() const {
    return _eepromSettings.loggerEnabled | false;
}

LoggerLevel Settings::loggerLevel() const {
    return static_cast<LoggerLevel>(_eepromSettings.loggerLevel);
}

char *Settings::customHotspotSsid() {
    String espDefaultName = ESP_DEFAULT_SSID;

    const String mac = WiFi.macAddress();
    espDefaultName += mac.substring(mac.length() - 6, mac.length());
    espDefaultName.replace(':', 'x');

    const bool useCustomSsid = _eepromSettings.hotspotSsid[0] != '\0';
    if (useCustomSsid && _eepromSettings.useCustomHSsid) {
        return _eepromSettings.hotspotSsid;
    }
    char *name = new char[espDefaultName.length() + 1];
    strcpy(name, espDefaultName.c_str());

    return const_cast<char *>(name);
}

void Settings::handleVersionChange(unsigned int currentFWVersion, bool requireEEPROMFormat) {
    const bool versionChanged = _eepromSettings.fwVersion != currentFWVersion;

    if (versionChanged && requireEEPROMFormat) {
        EEPROMSettings defaultEEPROMConfig;
        defaultEEPROMConfig.fwVersion = currentFWVersion;
        this->clearEeprom();
        this->writeToEEPROM(defaultEEPROMConfig);
        this->_eepromSettings = defaultEEPROMConfig;
    } else {
        this->_eepromSettings.fwVersion = currentFWVersion;
        this->writeToEEPROM(_eepromSettings);
    }
}

void Settings::clearEeprom() {
    logger.log("[Settings] Start clear EEPROM [1024] ...");
    EEPROM.begin(1024);
    for (int i = 0; i < 1024; ++i) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    EEPROM.end();
    logger.log("Done!");
}

String Settings::dataFromFS(const String &fileName) {
    String data;
    const char *file = fileName.c_str();

    bool success = SPIFFS.begin();
    if (success) {
        logger.log("[Settings][SPIFFS] File system mounted with success");

    } else {
        logger.log("[Settings][SPIFFS] Error mounting the dataFile system");
    }

    File dataFile = SPIFFS.open(file, "r");

    if (!dataFile) {
        logger.log("[Settings][SPIFFS] Error opening dataFile for writing. Creating new");
        File fileWrite = SPIFFS.open(file, "w");
        [[maybe_unused]] int bytesWritten = fileWrite.print("{}");
        fileWrite.close();
        return "{}";
    } else {
        while (dataFile.available()) {
            data += char(dataFile.read());
        }
        logger.log("[Settings][SPIFFS] File name: ", file);
        logger.logSerial("[Settings][SPIFFS] File data: ", data);
        dataFile.close();
        return data;
    }
}

void Settings::loadAdditionalData() {
    this->_dataFromFS = this->dataFromFS(FS_ADDITIONAL_DATA_PATH);
}

EEPROMSettings Settings::eepromSettings() const {
    return this->_eepromSettings;
}

JsonSettings Settings::eepromSettingsObj() const {
    JsonSettings settings;
    settings["useHotspotSsid"] = this->_eepromSettings.useCustomHSsid;
    settings["clientWebAccess"] = this->_eepromSettings.clientWebAccess;
    settings["loggerEnabled"] = this->_eepromSettings.loggerEnabled;
    settings["enableOtaUpdate"] = this->_eepromSettings.enableOtaUpdate;
    settings["loggerLevel"] = this->_eepromSettings.loggerLevel;
    settings["fwVersion"] = this->_eepromSettings.fwVersion;
    settings["wifiSsid"] = this->_eepromSettings.wifiSsid;
    settings["wifiPass"] = this->_eepromSettings.wifiPass;
    settings["hotspotSsid"] = this->_eepromSettings.hotspotSsid;
    return settings;
}
