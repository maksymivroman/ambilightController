//
// Created by rmaks on 11-Oct-23.
//

#include "Logger.h"


Logger::Logger(unsigned long serialBound, uint maxLogItems) :
        bound(serialBound), itemsCount(maxLogItems) {
}

void Logger::start(LoggerLevel mode) {
    this->loggerLevel = mode;
    if (canUseSerial()) {
        Serial.begin(this->bound);
        Serial.println();
        Serial.println("[Init logger]");
    }
    this->enabled = true;
}

void Logger::stop() {
    Serial.end();
    this->enabled = false;
}

bool Logger::canUseSerial() {
    return this->loggerLevel == SERIAL_AND_LOCAL || this->loggerLevel == LOGGER_SERIAL;
}

bool Logger::canUseLocal() {
    return this->loggerLevel == SERIAL_AND_LOCAL || this->loggerLevel == LOGGER_LOCAL;
}

bool Logger::canLog() const {
    return this->enabled;
}

std::vector<String> Logger::logs() const {
    return logsData;
}

void Logger::logMessage(const String &s, bool append) {
    String str = s;
    this->normalizeLogItem(str);
    this->messageBuffer += str;
    if (!append) {
        logSerial(this->messageBuffer);
        if (this->canUseLocal()) this->addMessageToLog(this->messageBuffer);
        this->messageBuffer = "";
    }
}

void Logger::logMessage(const char *str, bool append) {
    String s = str;
    this->normalizeLogItem(s);
    this->messageBuffer += s;
    if (!append) {
        logSerial(this->messageBuffer);
        if (this->canUseLocal()) this->addMessageToLog(this->messageBuffer);
        this->messageBuffer = "";
    }
}

void Logger::logMessage(const int &i, bool append) {
    String s = std::to_string(i).c_str();
    this->normalizeLogItem(s);
    this->messageBuffer += s;
    if (!append) {
        logSerial(this->messageBuffer);
        if (this->canUseLocal()) this->addMessageToLog(this->messageBuffer);
        this->messageBuffer = "";
    }
}

void Logger::logMessage(const JsonObject &j, bool append) {
    String s = "<unsupported log type(JsonObject)>";
    this->messageBuffer += s;
    if (!append) {
        logSerial(this->messageBuffer);
        if (this->canUseLocal()) this->addMessageToLog(this->messageBuffer);
        this->messageBuffer = "";
    }
}

void Logger::addMessageToLog(String &message) {
    if (this->logsData.size() == this->itemsCount) {
        this->logsData.erase(this->logsData.begin());
    }

    char formattedTime[9];
    millisToHMS(millis(), formattedTime);

    String messageToStore = "";
    messageToStore.concat(formattedTime);
    messageToStore.concat(" : ");
    messageToStore.concat(message);
    this->logsData.push_back(messageToStore);
}

void Logger::normalizeLogItem(String &message) {
    message.replace("'", "*");
    message.replace("\"", "**");
    message.replace("\\", "/");
    message.replace("{", "|");
    message.replace("}", "|");
}

void Logger::millisToHMS(unsigned long millisValue, char *formattedTime) {
    unsigned long seconds = millisValue / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    seconds %= 60;
    minutes %= 60;
    sprintf(formattedTime, "%02lu:%02lu:%02lu", hours, minutes, seconds);
}

