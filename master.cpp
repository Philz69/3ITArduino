#include "master.h"

Master::Master() {
    Serial.begin(baudRate);
}

String Master::getCommand() {
    String serialIn = Serial.readStringUntil('\n');
    Serial.println(serialIn);
    return serialIn;
}

void Master::sendData(Channels channels) {
    DynamicJsonDocument doc(2048);
    JsonObject JSONchannels = doc.createNestedObject("channels");
    JsonArray JSONtemperatureChannels = JSONchannels.createNestedArray("TemperatureChannels");
    JsonArray JSONpassiveChannels = JSONchannels.createNestedArray("passiveChannels");
    JsonArray JSONactiveChannels = JSONchannels.createNestedArray("ActiveChannels");
    JsonObject JSONsweepResult = JSONactiveChannels.createNestedObject();
    JsonArray JSONsweepResults = JSONsweepResult.createNestedArray("sweepResult");
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
        for(int j = 0; j < 255; j++)
        {
            JsonObject data = JSONsweepResults.createNestedObject();
            data["voltage"] = channels.ActiveChannels[i]->getSweepResult(j).voltage;
            data["current"] = channels.ActiveChannels[i]->getSweepResult(j).current;
        }
    }
    unsigned long initTime = millis();
    serializeJson(doc, Serial);
    unsigned long printTime = millis();
    Serial.println();
    unsigned long ignoreTime = millis();

}