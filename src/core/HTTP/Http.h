//
// Created by rmaks on 16-May-24.
//

#ifndef TEMPLATE_ESP8266_CORE_HTTP_H
#define TEMPLATE_ESP8266_CORE_HTTP_H

#include <Arduino.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "core/Settings/Settings.h"

class Http {

public:
    static String statusError(const String &message);
    static String statusOk();
    static String statusOk(const String &message);
    static String statusOk(const String &message, const String &result);
    static String statusInfo(const String &message);

    static String dataResponse(const String &message, const JsonSettings &data);
    static String dataResponse(const String &message, const String &result);
    static String dataResponse(const String &message, const JsonObject &result);

    template<typename T>
    static String dataResponse(const String &message, const T &src, size_t size){
        StaticJsonDocument<512> jsonDoc;
        copyArray(src, size, jsonDoc);
        String result;
        serializeJson(jsonDoc, result);
        StaticJsonDocument<512> doc;
        auto respObj = doc.to<JsonObject>();
        respObj["status"] = "OK";
        respObj["message"] = message;
        respObj["result"] = jsonDoc;
        return responseToString(respObj);
    }

private:
    static String responseToString(JsonObject &obj);

};


#endif //TEMPLATE_ESP8266_CORE_HTTP_H
