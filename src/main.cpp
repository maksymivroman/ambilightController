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
#include "core/RGB/RGB.hpp"

#include <FastLED.h>
#define DATA_PIN 4
#define COLOR_ORDER GRB

volatile int ledsCount = 1;

CRGB *LedStrip;

Version deviceVersion(0, 0, 0, true);
const byte pinA1 = 5;

StartupMode deviceStartupMode = RUN;

Settings deviceSettings;
AsyncWebServer server(80);
NetworkService networkService;
Trigger RestartTrigger, UpdateRGBTrigger, TestRGB, FadeOutRgbTrigger;

OtaUpdate otaUpdate(&RestartTrigger);

Logger logger(115200, 20);

Task restartTask, checkConnectionTask, updateRGB, testRGB, fadeOutRGB;
IntervalTask Main;

void restart() {
    optimistic_yield(1000);
    EspClass::restart();
}

void setStartupMode() {
    /** define mode to start here */
    StartupMode mode = digitalRead(pinA1) ? SETUP : RUN;
    logger.log("[Init] Startup mode: ", mode);
    deviceStartupMode = mode;
}

uint32_t parseHexColor(const String& hex) {
    if (hex.isEmpty()) return 0;
    // Пропускаємо символ '#', якщо він є (індекс 1), інакше 0
    int offset = (hex[0] == '#') ? 1 : 0;
    return strtoul(hex.c_str() + offset, nullptr, 16);
}

void ledStripTest(unsigned int ledCount) {
    logger.log("[STRIP TEST] Started. LED count: ", ledCount);
    for (int whiteLed = 0; whiteLed < ledCount; whiteLed = whiteLed + 1) {
        LedStrip[whiteLed] = CRGB::White;
        FastLED.show();
        delay(50);
        LedStrip[whiteLed] = CRGB::Black;
    }
}

void fadeOut(unsigned int ledCount) {
    // Робимо цикл, щоб поступово зменшувати яскравість
    // Достатньо близько 30-50 кроків, щоб повністю погасити навіть яскраві кольори
    for (int i = 0; i < 40; i++) {

        // fadeToBlackBy(масив, кількість, на_скільки_зменшити)
        // 10 - це крок згасання (від 0 до 255).
        // Чим менше число, тим плавніше згасання.
        // Чим більше число, тим різкіше.
        fadeToBlackBy(LedStrip, ledCount, 15);

        FastLED.show();

        // Затримка між кадрами анімації (в мілісекундах)
        // Впливає на загальну тривалість згасання
        delay(30);
    }

    // Гарантуємо, що в кінці все точно вимкнено (прибираємо залишкові тьмяні кольори)
    FastLED.clear();
    FastLED.show();
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

    /** if needed load data stored in FS
    deviceSettings.loadData();
     */

    WiFiSettings wiFiConnDetails = deviceSettings.wifiSettings();
    [[maybe_unused]] EEPROMSettings deviceConfiguration = deviceSettings.eepromSettings();
    Networks wiFiList = networkService.wiFiList();

    if (deviceStartupMode == SETUP) {
        const char *networkSsid = deviceSettings.customHotspotSsid();
        networkService.buttonHotspot(true, networkSsid, DEVICE_HOTSPOT_PASS);
    } else if (deviceStartupMode == RUN) {
        networkService.connectToWiFi(wiFiConnDetails);
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


        logger.log("[Init RGB Strip]");
        logger.log("[Init RGB Strip] LED COUNT: ", deviceSettings.ledCount());
        LedStrip = new CRGB[deviceSettings.ledCount()];
        logger.log("[Init RGB Strip] PIN: ", DATA_PIN);
        FastLED.addLeds<WS2811, DATA_PIN, COLOR_ORDER>(LedStrip, deviceSettings.ledCount());
//    FastLED.setBrightness(100);
        FastLED.clear();
        FastLED.show();
        logger.log("[Init RGB Strip successful]");



        auto *rgbHandler = new AsyncCallbackJsonWebHandler("/rgb",
                                                        [](AsyncWebServerRequest *request, JsonVariant &json) {
            if (json.is<JsonObject>()) {
                JsonObject jsonObj = json.as<JsonObject>();
                auto flowDirection = deviceSettings.ledFlowDirection();
                auto sides = RgbFlowDirections.at(flowDirection);
                int currentLedIndex = 0;
                for (const char* side : sides) {
                    JsonArray array = jsonObj[side];
                    if (array.isNull()) continue; // Якщо сторони немає в JSON, пропускаємо

                    for (String colorStr: array) {
                        // Захист від переповнення буфера (якщо прийшло більше кольорів, ніж є діодів)
                        if (currentLedIndex >= deviceSettings.ledCount()) break; //todo -> set NUM_LEDS from total colors received

                        LedStrip[currentLedIndex] = parseHexColor(colorStr);

                        currentLedIndex++;
                    }
                }
                UpdateRGBTrigger.set();
                request->send(200, "application/json", Http::statusOk("ok"));
            } else {
                request->send(400, "application/json", Http::statusError("wrong data") );
            }
        }, 8192);

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

        if (deviceStartupMode == SETUP || (deviceStartupMode == RUN && deviceSettings.otaUpdateOnClientMode())) {
            otaUpdate.init(deviceSettings.customHotspotSsid(), DEVICE_HOSTNAME);
            otaUpdate.begin(&server);
        }

        server.addHandler(handler);
        server.addHandler(rgbHandler);
        server.begin();
    }

    logger.logSerial("[Init successful]");
}

void loop() {
    Main(200, [](){
        /**Place your code here, adjust interval*/
        testRGB(TestRGB.get(), [](){
            ledStripTest(deviceSettings.ledCount());
            TestRGB.reset();
        });
    });

    updateRGB(UpdateRGBTrigger.get(), [](){
        FastLED.show();
        UpdateRGBTrigger.reset();
    });

    fadeOutRGB(FadeOutRgbTrigger.get(), []() {
        fadeOut(deviceSettings.ledCount());
        FadeOutRgbTrigger.reset();
    });

    restartTask(RestartTrigger.get(), restart);
}
