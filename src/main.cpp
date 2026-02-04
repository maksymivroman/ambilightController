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
#include "core/OTAUpdate/OTAUpdate.h"
#include "core/Trigger/Trigger.h"

Version deviceVersion(0, 0, 1, true);
const byte pinA1 = 5;

StartupMode deviceStartupMode = RUN;

Settings deviceSettings;
AsyncWebServer server(80);
NetworkService networkService;
Trigger RestartTrigger;

OtaUpdate otaUpdate(&RestartTrigger);

Logger logger(115200, 20);

Task restartTask, checkConnectionTask;
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
        server.begin();
    }

    logger.logSerial("[Init successful]");
}

void loop() {
    Main(1000, [](){
        /**Place your code here, adjust interval*/
    });

    restartTask(RestartTrigger.get(), restart);
}
