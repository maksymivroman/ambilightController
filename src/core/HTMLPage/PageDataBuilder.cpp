//
// Created by rmaks on 12-Apr-24.
//

#include "PageDataBuilder.h"
#include "core/Version/Version.h"
#include "core/Global/Global.hpp"

extern Version deviceVersion;

String PageDataBuilder::pageDataByRef(const String &ref) {
    String data = "";
    if (ref == "VERSION") {
        data += deviceVersion.str_fullVersion();
        return data;
    } else if (ref == "PAGE_TITLE") {
        data += DEVICE_HOSTNAME;
        return data;
    } else if (ref == "PAGE_SECTION") {
        data += "settings";
        return data;
    }
    return data;
}
