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
    settings.saveLastState = jsonSettings["saveLastState"].as<bool>() | false;
    settings.loggerLevel = jsonSettings["loggerLevel"].as<unsigned int>() | 0;
    settings.ledCount = jsonSettings["ledCount"].as<unsigned int>() | 1;
    settings.ledFlowDirection = static_cast<RGBDirection>(jsonSettings["ledFlowDirection"].as<RGBDirection>() | TRBL);
    settings.wiFiMode = static_cast<CONTROLLER_WIFI_MODE>(jsonSettings["wiFiMode"].as<CONTROLLER_WIFI_MODE>() | AUTO);

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

bool Settings::saveLastState() const {
    return _eepromSettings.saveLastState | false;
}

unsigned int Settings::ledCount() const {
    return _eepromSettings.ledCount | 1;
}

LoggerLevel Settings::loggerLevel() const {
    return static_cast<LoggerLevel>(_eepromSettings.loggerLevel);
}

RGBDirection Settings::ledFlowDirection() const {
    return static_cast<RGBDirection>(_eepromSettings.ledFlowDirection | TRBL);
}

String Settings::name() const {
    return this->_eepromSettings.hotspotSsid;
}

char *Settings::customHotspotSsid() {
    const bool useCustomSsid = _eepromSettings.hotspotSsid[0] != '\0';
    if (useCustomSsid && _eepromSettings.useCustomHSsid) {
        return _eepromSettings.hotspotSsid;
    }
    return const_cast<char *>(this->defaultDeviceName());
}

char *Settings::defaultDeviceName() {
    String espDefaultName = ESP_DEFAULT_SSID;

    const String mac = WiFi.macAddress();
    espDefaultName += mac.substring(mac.length() - 6, mac.length());
    espDefaultName.replace(':', 'x');

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
    settings["saveLastState"] = this->_eepromSettings.saveLastState;
    settings["ledCount"] = this->_eepromSettings.ledCount;
    settings["ledFlowDirection"] = this->_eepromSettings.ledFlowDirection;
    settings["wiFiMode"] = this->_eepromSettings.wiFiMode;
    return settings;
}

CONTROLLER_WIFI_MODE Settings::wiFiMode() const {
    return this->_eepromSettings.wiFiMode;
}

void Settings::saveCurrentState(LEDState &stateColors) {
    File file = SPIFFS.open(FS_LED_STATE_PATH, "w");
    if (!file) {
        logger.log("[Settings][SPIFFS] Error opening file for writing. File not exist.");
        if (!stateColors.empty()) {
            logger.log("[Settings][SPIFFS] Creating new file...");
            file.println();
        }
    } else {
        if (!stateColors.empty()) {
            logger.log("[Settings][SPIFFS] Updating file...");
            file.write(reinterpret_cast<uint8_t*>(stateColors.data()), stateColors.size() * sizeof(CRGB));
        }
    }

    file.close();
    logger.log("[Settings][SPIFFS] Colors saved successfully.");
}

LEDState Settings::getLastState() {
    LEDState stateColors;

    if (!SPIFFS.exists(FS_LED_STATE_PATH)) {
        logger.log("[Settings][SPIFFS] No saved colors found.");
        return stateColors;
    }

    File file = SPIFFS.open(FS_LED_STATE_PATH, "r");
    if (!file) {
        logger.log("[Settings][SPIFFS] Failed to open file for reading!");
        return stateColors;
    }

    size_t fileSize = file.size();
    size_t numColors = fileSize / sizeof(CRGB);

    stateColors.resize(numColors);

    if (numColors > 0) {
        file.read(reinterpret_cast<uint8_t*>(stateColors.data()), fileSize);
    }

    file.close();
    logger.log("[Settings][SPIFFS] Loaded colors from memory. ", numColors);

    return stateColors;
}

void Settings::saveCurrentBrightness(unsigned int brightness) {
    File file = SPIFFS.open(FS_LED_BRIGHTNESS_PATH, "w");
    if (!file) {
        logger.log("[Settings][SPIFFS] Error opening brightness file for writing!");
        logger.log("[Settings][SPIFFS] Creating new file...");
        file.println();
    } else {
        file.write(reinterpret_cast<const char *>(&brightness), sizeof(unsigned int));
    }

    file.close();
    logger.log("[Settings][SPIFFS] Brightness saved successfully.");
}

unsigned int Settings::getLastBrightness() {
    unsigned int brightness = 255;

    if (!SPIFFS.exists(FS_LED_BRIGHTNESS_PATH)) {
        logger.log("[Settings][SPIFFS] Error opening brightness file for reading!");
        return brightness;
    }

    File file = SPIFFS.open(FS_LED_BRIGHTNESS_PATH, "r");
    if (!file) return brightness;

    file.read(reinterpret_cast<uint8_t *>(&brightness), sizeof(unsigned int));
    file.close();

    return brightness;
}
