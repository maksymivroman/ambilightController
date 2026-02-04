//
// Created by rmaks on 16-Dec-24.
//

#ifndef TEMPLATE_ESP8266_CORE_OTAUPDATE_H
#define TEMPLATE_ESP8266_CORE_OTAUPDATE_H

#endif //TEMPLATE_ESP8266_CORE_OTAUPDATE_H

#include <Arduino.h>

#include "stdlib_noniso.h"

#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#include "flash_hal.h"
#include "FS.h"

#include "Hash.h"
#include "ESPAsyncWebServer.h"
#include "FS.h"

#include "core/Global/Global.hpp"
#include "core/Logger/Logger.h"
#include "core/Version/Version.h"
#include "core/Trigger/Trigger.h"

#include "UpdateWebPage.h"

extern Logger logger;
extern Version deviceVersion;

class OtaUpdate {


public:

    OtaUpdate(Trigger * restartHandler) {
        this->restartHandler = restartHandler;
    }

    void init(const char* deviceId, const char* pageTitle){
        this->_id = deviceId;
        this->_title = pageTitle;
    }

    void begin(AsyncWebServer *server, const char* username = "", const char* password = ""){
        _server = server;

        if(strlen(username) > 0){
            _authRequired = true;
            _username = username;
            _password = password;
        }else{
            _authRequired = false;
            _username = "";
            _password = "";
        }

        _server->on("/update/identity", HTTP_GET, [&](AsyncWebServerRequest *request){
            if(_authRequired){
                if(!request->authenticate(_username.c_str(), _password.c_str())){
                    return request->requestAuthentication();
                }
            }
            request->send(200, "application/json",
                          R"({"id": ")" + _id + R"(", "title": ")" + _title + R"(", "firmware": ")" + _firmware +
                          "\"}");

        });

        _server->on("/update", HTTP_GET, [&](AsyncWebServerRequest *request){
            if(_authRequired){
                if(!request->authenticate(_username.c_str(), _password.c_str())){
                    return request->requestAuthentication();
                }
            }
            AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", UPDATE_HTML, UPDATE_HTML_SIZE);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        });

        _server->on("/update", HTTP_POST, [&](AsyncWebServerRequest *request) {
            logger.log("[UPDATE] Upload...");
            if(_authRequired){
                if(!request->authenticate(_username.c_str(), _password.c_str())){
                    return request->requestAuthentication();
                }
            }
            // the request handler is triggered after the upload has finished...
            // create the response, add header, and send response
            AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");
            response->addHeader("Connection", "close");
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
            restart();
        }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            //Upload handler chunks in data
            if(_authRequired){
                if(!request->authenticate(_username.c_str(), _password.c_str())){
                    return request->requestAuthentication();
                }
            }

            if (!index) {
                if(!request->hasParam("MD5", true)) {
                    return request->send(400, "text/plain", "MD5 parameter missing");
                }

                if(!Update.setMD5(request->getParam("MD5", true)->value().c_str())) {
                    return request->send(400, "text/plain", "MD5 parameter invalid");
                }

                int cmd = (filename == "filesystem") ? U_FS : U_FLASH;
                Update.runAsync(true);
                size_t fsSize = ((size_t) &_FS_end - (size_t) &_FS_start);
                uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
                if (!Update.begin((cmd == U_FS)?fsSize:maxSketchSpace, cmd)){ // Start with max available size

                    Update.printError(Serial);
                    return request->send(400, "text/plain", "OTA could not begin");
                }
            }

            // Write chunked data to the free sketch space
            if(len){
                if (Update.write(data, len) != len) {
                    return request->send(400, "text/plain", "OTA could not begin");
                }
            }

            if (final) { // if the final flag is set then this is the last frame of data
                if (!Update.end(true)) { //true to set the size to the current progress
                    Update.printError(Serial);
                    return request->send(400, "text/plain", "Could not end OTA");
                }
            }else{
                return;
            }
        });
    }

    void restart() {
        logger.log("[FW UPDATE] Restart...");
        this->restartHandler->set();
    }

private:
    AsyncWebServer *_server;
    Trigger *restartHandler;

    String getFW(){
        return deviceVersion.str_fullVersion();
    }

    String _title = "";
    String _id = "";
    String _firmware = getFW();
    String _username = "";
    String _password = "";
    bool _authRequired = false;

};