#include <Arduino.h>

#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "core/Version/Version.h"
#include "core/Settings/Settings.h"
#include "core/TasksHandler/Task.h"
#include "core/NetworkService/NetworkService.h"
#include "core/HTMLPage/PageDataBuilder.h"
#include "core/Http/Http.h"
#include "core/HTMLPage/html-page.hpp"
#include "core/HTMLPage/colorPicker-page.hpp"
#include "core/OTAUpdate/OTAUpdate.h"
#include "core/Trigger/Trigger.h"
#include "core/LEDService/LEDStripService.h"
#include "core/ID/ID.h"

Version deviceVersion(1, 0, 0, false);
const byte pinA1 = 4;

StartupMode deviceStartupMode = RUN;

Settings deviceSettings;
AsyncWebServer server(80);
NetworkService networkService;
Trigger RestartTrigger, UpdateRGBTrigger, TestRGB, FadeOutRgbTrigger, ControlButtonState;

OtaUpdate otaUpdate(&RestartTrigger);

Logger logger(115200, 20);

Task restartTask, testRGB, fadeOutRGB, toggleStripTask(true), CheckConnectionTask(true);
IntervalTask Main, ConnectToWiFiITask;

LEDStripService LEDService;

void restart() {
    optimistic_yield(1000);
    EspClass::restart();
}

void setStartupMode() {
    /** define mode to start here */
    StartupMode mode = digitalRead(pinA1) ? RUN : SETUP;
    logger.log("[Init] Startup mode: ", mode);
    deviceStartupMode = mode;
}

uint32_t parseHexColor(const String& hex) {
    if (hex.isEmpty()) return 0;
    // Пропускаємо символ '#', якщо він є (індекс 1), інакше 0
    int offset = (hex[0] == '#') ? 1 : 0;
    return strtoul(hex.c_str() + offset, nullptr, 16);
}

void setup() {
    /** define IO pins here */
    pinMode(pinA1, INPUT);

    /** load device settings and init device*/
    deviceSettings.loadSettings();
    deviceSettings.handleVersionChange(
            deviceVersion.uint_version(),
            deviceVersion.EEPROMStructureChanged());
    if (deviceSettings.loggerEnabled()) {
        logger.start(deviceSettings.loggerLevel());
        logger.log("CURRENT FW: " , deviceVersion.str_fullVersion());
    }
    setStartupMode();

    deviceSettings.loadAdditionalData();

    WiFiSettings wiFiConnDetails = deviceSettings.wifiSettings();
    [[maybe_unused]] EEPROMSettings deviceConfiguration = deviceSettings.eepromSettings();
    Networks wiFiList = networkService.wiFiList();
    networkService.setWiFiMode(deviceSettings.wiFiMode());

    if (deviceStartupMode == SETUP) {
        const char *networkSsid = deviceSettings.customHotspotSsid();
        networkService.buttonHotspot(true, networkSsid, DEVICE_HOTSPOT_PASS);
    }

    LEDService.init(deviceSettings.ledCount());
    auto ledState = deviceSettings.getLastState();
    auto ledBrightness = deviceSettings.getLastBrightness();
    if (deviceSettings.saveLastState() && !ledState.empty()) {
        LEDService.initColorState(ledState);
        LEDService.setBrightness(ledBrightness);
    } else {
        LEDService.initColorState(GhostWhite);
        LEDService.setBrightness(255);
    }

    if (deviceSettings.clientWebAccessEnabled() || deviceStartupMode == SETUP) {
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, "text/html", index_html,
                            [](const String& ref){ return PageDataBuilder::pageDataByRef(ref);});
        });

        server.on("/color", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, "text/html", colorPicker_html);
        });

        server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
            auto data = deviceSettings.eepromSettingsObj();
            request->send(200, "application/json", Http::dataResponse("success", data));
        });

        auto *handler = new AsyncCallbackJsonWebHandler("/settings",
                                                        [](AsyncWebServerRequest *request, JsonVariant &json) {
            DynamicJsonDocument data(4096);
            if (json.is<JsonObject>())            {
                data = json.as<JsonObject>();
                auto result = deviceSettings.saveSettings(data);
                if (result == DATA_SAVED) {
                    request->send(200, "application/json", Http::statusOk("Data saved"));
                } else {
                    request->send(400, "application/json", Http::statusError("Failed to save"));
                }
            } else {
                request->send(400, "application/json", Http::statusError("Failed to save, wrong data") );
            }
        });


        /**
         * expected payload:
         * {
         *  top: string[]       //(rgb);
         *  right: string[]     //(rgb);
         *  bottom: string[]    //(rgb);
         *  left: string[]      //(rgb);
         * }
         * */

        auto *rgbHandler = new AsyncCallbackJsonWebHandler("/rgb",
                                                        [](AsyncWebServerRequest *request, JsonVariant &json) {
            if (json.is<JsonObject>()) {

                JsonObject jsonObj = json.as<JsonObject>();
                auto flowDirection = deviceSettings.ledFlowDirection();
                auto sides = &RgbFlowDirections.at(flowDirection);
                LEDState stateColors;
                stateColors.clear();
                stateColors.reserve(deviceSettings.ledCount());
                int currentLedIndex = 0;
                for (const char* side : *sides) {
                    JsonArray array = jsonObj[side];
                    if (array.isNull()) continue;
                    for (String colorStr: array) {
                        if (currentLedIndex >= deviceSettings.ledCount()) break;
                        stateColors.emplace_back(parseHexColor(colorStr));
                        currentLedIndex++;
                    }
                }
                LEDService.setColors(stateColors);
                deviceSettings.saveCurrentState(stateColors);
                request->send(200, "application/json", Http::statusOk("ok"));
            } else {
                request->send(400, "application/json", Http::statusError("wrong data") );
            }
        }, 8192);

        auto *brightnessHandler = new AsyncCallbackJsonWebHandler("/brightness", [](AsyncWebServerRequest *request, JsonVariant &json) {
            if (json.is<JsonObject>()) {
                JsonObject jsonObj = json.as<JsonObject>();
                if (jsonObj.containsKey("brightness")) {
                    unsigned int brightnessValue = jsonObj["brightness"].as<unsigned int>();
                    LEDService.setBrightness(brightnessValue);
                    deviceSettings.saveCurrentBrightness(brightnessValue);
                    request->send(200, "application/json", Http::statusOk("ok"));
                } else {
                    request->send(400, "application/json", Http::statusError("Missing brightness data") );
                }
            } else {
                request->send(400, "application/json", Http::statusError("Invalid JSON") );
            }
        }, 1024);

        server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
            ControlButtonState.set();
            LEDService.clear();
            request->send(200, "application/json", Http::statusOk("Test RGB"));
        });

        server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request) {
            DynamicJsonDocument payloadDoc(2048);
            JsonArray colorsArray = payloadDoc.createNestedArray("colors");
            payloadDoc["brightness"] = LEDService.getBrightness();
            for (const CRGB& color : LEDService.currentState()) {
                char hexBuf[8];
                snprintf(hexBuf, sizeof(hexBuf), "#%02x%02x%02x", color.r, color.g, color.b);
                colorsArray.add(hexBuf);
            }
            request->send(200, "application/json", Http::dataResponse<3072>("Current state", payloadDoc.as<JsonObject>()));
        });

        server.on("/testRgb", HTTP_GET, [](AsyncWebServerRequest *request) {
            TestRGB.set();
            request->send(200, "application/json", Http::statusOk("Test RGB"));
        });

        server.on("/fadeOutRgb", HTTP_GET, [](AsyncWebServerRequest *request) {
            FadeOutRgbTrigger.set();
            request->send(200, "application/json", Http::statusOk("Fade Out RGB"));
        });

        server.on("/networks", HTTP_GET, [wiFiList](AsyncWebServerRequest *request) {
            request->send(200, "application/json", Http::dataResponse("Available networks", wiFiList.arr, wiFiList.size));
        });

        server.on("/deviceMode", HTTP_GET, [](AsyncWebServerRequest *request) {
            StaticJsonDocument<16> response;
            JsonObject responseObject = response.to<JsonObject>();
            responseObject["startupMode"] = deviceStartupMode == RUN ? "RUN" : "SETUP";
            request->send(200, "application/json", Http::dataResponse("Device startup mode", responseObject));
        });

        server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
            auto *response = new AsyncJsonResponse(false, 8192);
            JsonObject root = response->getRoot();
            const unsigned int logsCount = logger.logs().size();
            for (size_t i = 0; i < logsCount; ++i) {
                root[std::to_string(i)] = logger.logs()[i];
            }
            response->setLength();
            request->send(response);
        });

        server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
            restartTask.executeNext();
            request->send(200, "application/json", Http::statusOk());
        });

        server.on("/id", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "application/json", ID::toJsonString());
        });
        ID::setUrl("/", "/color");
        ID::addAction("Restart", "/restart");
        ID::addAction("Test RGB", "/testRgb");

        if (deviceStartupMode == SETUP || (deviceStartupMode == RUN && deviceSettings.otaUpdateOnClientMode())) {
            otaUpdate.init(deviceSettings.customHotspotSsid(), DEVICE_HOSTNAME);
            otaUpdate.begin(&server);
        }

        server.addHandler(handler);
        server.addHandler(rgbHandler);
        server.addHandler(brightnessHandler);
        server.begin();
    }

    logger.logSerial("[Init successful]");
}

void loop() {
    ConnectToWiFiITask(1000, []() {
        auto config = deviceSettings.wifiSettings();
        networkService.connectToWiFi(config);
    }, (deviceStartupMode == SETUP) || networkService.isConnectedToWiFi());

    CheckConnectionTask(
            networkService.isConnectedToWiFi(),
            [](){ logger.log("[CheckConnectionTask]: Connected. IP: ", networkService.ipAddress());
            },
            [](){ logger.log("[CheckConnectionTask]: Disconnected"); },
            deviceStartupMode == SETUP
    );

    Main(200, [](){
        /**Place your code here, adjust interval*/
        testRGB(TestRGB.get(), [](){
            LEDService.testLEDStrip();
            TestRGB.reset();
        });
        toggleStripTask(digitalRead(pinA1),
                        []() {},
                        []() {
                            logger.log("[MAIN] Toggle light ", ControlButtonState.get());
                            if (!ControlButtonState.get()) {
                                LEDService.restoreLastState();
                            } else {
                                LEDService.clear();
                            }
                            ControlButtonState = !ControlButtonState.get();
        });
    });

    fadeOutRGB(FadeOutRgbTrigger.get(), []() {
        LEDService.fadeOut();
        FadeOutRgbTrigger.reset();
    });

    restartTask(RestartTrigger.get(), [](){
        LEDService.fillColor(Red);
        restart();
    });
}
