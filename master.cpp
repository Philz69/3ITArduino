#include "master.h"

Master::Master() {
}

String Master::getCommand() {
    String serialIn = Serial.readStringUntil('\n');
    return serialIn;
}

void Master::sendUpdate(Channels &channels) {
    DynamicJsonDocument doc(2048);
    doc["time"] = millis();
    JsonObject JSONchannels = doc.createNestedObject("channels");
    JsonArray JSONtemperatureChannels = JSONchannels.createNestedArray("TemperatureChannels");
    JsonArray JSONpassiveChannels = JSONchannels.createNestedArray("PassiveChannels");
    JsonArray JSONactiveChannels = JSONchannels.createNestedArray("ActiveChannels");
    for(int i = 0; i < 16; i++)
    {
        JsonObject data = JSONtemperatureChannels.createNestedObject();
        data["temperature"] = channels.TemperatureChannels[i]->getTemperature();
    }
    for(int i =0; i < 8; i ++) {
        JsonObject data = JSONpassiveChannels.createNestedObject();
        data["voltage"] = channels.PassiveChannels[i]->getVoltage();
        data["current"] = channels.PassiveChannels[i]->getCurrent(); 
    }
    for(int i = 0; i < 8; i++)
    {
        JsonObject data = JSONactiveChannels.createNestedObject();
        data["voltage"] = channels.ActiveChannels[i]->getVoltage();
        data["current"] = channels.ActiveChannels[i]->getCurrent(); 
    }
    serializeJson(doc, Serial);
    Serial.println();
}

void Master::sendPartialSweepData(sweepResult sweepResult[32]) {
}
