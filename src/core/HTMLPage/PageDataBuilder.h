//
// Created by rmaks on 12-Apr-24.
//

#ifndef TEMPLATE_ESP8266_CORE_PAGEDATABULDER_H
#define TEMPLATE_ESP8266_CORE_PAGEDATABULDER_H

#include <Arduino.h>

#include "core/NetworkService/NetworkService.h"

extern NetworkService Network;

class PageDataBuilder {

public:
    static String pageDataByRef(const String& ref);

private:
    String networksDataObj;
};


#endif //TEMPLATE_ESP8266_CORE_PAGEDATABULDER_H
