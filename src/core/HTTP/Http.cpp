//
// Created by rmaks on 16-May-24.
//

#include "Http.h"

String Http::statusError(const String& message) {
    StaticJsonDocument<512> doc;
    auto respObj = doc.to<JsonObject>();
    respObj["status"] = "error";
    respObj["message"] = message;
    return responseToString(respObj);
}

String Http::statusOk() {
    StaticJsonDocument<512> doc;
    auto respObj = doc.to<JsonObject>();
    respObj["status"] = "OK";
    return responseToString(respObj);
}

String Http::statusOk(const String& message) {
    StaticJsonDocument<512> doc;
    auto respObj = doc.to<JsonObject>();
    respObj["status"] = "OK";
    respObj["message"] = message;
    return responseToString(respObj);
}

String Http::statusOk(const String &message, const String &result) {
    StaticJsonDocument<512> doc;
    auto respObj = doc.to<JsonObject>();
    respObj["status"] = "OK";
    respObj["message"] = message;
    respObj["result"] = result;
    return responseToString(respObj);;
};

String Http::statusInfo(const String& message) {
    StaticJsonDocument<512> doc;
    auto respObj = doc.to<JsonObject>();
    respObj["status"] = "OK";
    respObj["message"] = message;
    return responseToString(respObj);
}

String Http::responseToString(JsonObject &obj) {
    String response;
    serializeJson(obj, response);
    return response;
}

String Http::dataResponse(const String &message, const String &result) {
    StaticJsonDocument<512> doc;
    auto respObj = doc.to<JsonObject>();
    respObj["status"] = "OK";
    respObj["message"] = message;
    respObj["result"] = result;
    return responseToString(respObj);
}

String Http::dataResponse(const String &message, const JsonSettings &data) {
    String result;
    serializeJson(data, result);
    StaticJsonDocument<512> doc;
    auto respObj = doc.to<JsonObject>();
    respObj["status"] = "OK";
    respObj["message"] = message;
    respObj["result"] = data;
    return responseToString(respObj);
}

String Http::dataResponse(const String &message, const JsonObject &result) {
    StaticJsonDocument<512> doc;
    auto respObj = doc.to<JsonObject>();
    respObj["status"] = "OK";
    respObj["message"] = message;
    respObj["result"] = result;
    return responseToString(respObj);
}
