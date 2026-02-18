
#include "ID.h"


ID::Action ID::_actions[ID::MAX_ACTIONS];
const char *ID::_type = "CONTROLLER";
const char *ID::_id = "AMBILIGHT";
String ID::_hostUrl = "/";
String ID::_uiUrl = "/";
String ID::_widgetUrl = "";
size_t ID::_actionCount = 0;

void ID::addAction(const String &name, const String &action) {
    if (_actionCount < MAX_ACTIONS) {
        _actions[_actionCount++] = {name, action};
    }
}

String ID::toJsonString() {
    StaticJsonDocument<1024> doc;

    doc["type"] = _type;
    doc["id"] = _id;
    doc["wirelessMac"] = WiFi.macAddress();
    doc["name"] = deviceSettings.name();
    doc["hostUrl"] = _hostUrl;
    doc["uiUrl"] = _uiUrl;
    doc["defaultSSID"] = deviceSettings.defaultDeviceName();
    doc["customSSID"] = deviceSettings.customHotspotSsid();
    doc["fw"] = deviceVersion.str_version();
    doc["hw"] = "unknown";
    if (_widgetUrl.isEmpty()) {
        doc["widgetUrl"] = nullptr;
    } else {
        doc["widgetUrl"] = _widgetUrl;
    }
    JsonArray actionsArray = doc.createNestedArray("actions");

    for (size_t i = 0; i < _actionCount; i++) {
        JsonObject actionObj = actionsArray.createNestedObject();
        actionObj["name"] = _actions[i].name;
        actionObj["action"] = _actions[i].action;
    }

    String output;
    serializeJson(doc, output);
    return output;
}

void ID::setUrl(const String &hostUrl, const String &uiUrl) {
    _hostUrl = hostUrl;
    _uiUrl = uiUrl;
}

void ID::setUrl(const String &hostUrl, const String &uiUrl, const String &widgetUrl) {
    _hostUrl = hostUrl;
    _uiUrl = uiUrl;
    _widgetUrl = widgetUrl;
}
