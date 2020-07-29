#include <arduino.h>
#include "channels.h"
#include "master.h"
#include <ArduinoJson.h>
#include <SPI.h>
int ledPin = 13;

int pinCS1 = 22;
int pinCS2 = 23;
int pinCS3 = 24;
int pinCS4 = 25;
int pinCS5 = 25;
int pinCS6 = 26;
int pinCS7 = 27;
int pinCS8 = 28;
int pinCS9 = 29;
int pinCS10 = 30;
int pinCS11 = 31;
int pinCS12 = 32;
int pinCS13 = 33;

int voltagePassif1 = 54;
int voltage = 0;

int lastUpdate = millis();
const int updateTime = 1000;
//Channels channels;
Channels channels;
Master raspberrypi;

bool sweetOnce = true;

void setup()
{
    digitalWrite(LED_BUILTIN,LOW);
    Serial.begin(115200);
    SPI.begin();
    for(int i = 0; i < 8; i++)
    {
        channels.TemperatureChannels[i]->init();
        channels.ActiveChannels[i]->startMPPT();
    }
    setPWMScaler(1);
    pinMode(ledPin, OUTPUT);
    Serial.println("Ready");
}

void loop() { 
    /*if((millis() - lastUpdate) > updateTime) {
        updateAllChannels();
        lastUpdate = millis();
    }*/

    for(int i = 0; i < 8; i++)
    {
        int mode = channels.ActiveChannels[i]->getMode();
        if(mode == MPPT_MODE)
        {
            channels.ActiveChannels[i]->updateMPPT();
        }
        else if(mode == SWEEP_MODE)
        {
            channels.ActiveChannels[i]->sweepIVasync();
        }
    }

    execCommand(raspberrypi.getCommand());

}
void setPWMScaler(int value) {
    if(value < 1 || value > 6)
    {
        return;
    }
    int bitEraser = 7;
    TCCR1B &= ~bitEraser; 
    TCCR2B &= ~bitEraser; 
    TCCR3B &= ~bitEraser;
    TCCR4B &= ~bitEraser;
    TCCR1B |= value;
    TCCR2B |= value;
    TCCR3B |= value;
    TCCR4B |= value;
}

void updateAllChannels() {
        for(int i = 0 ; i < 8; i++)
        {
            channels.ActiveChannels[i]->updateMPPT();
        }
        for(int i=0; i < 8; i++)
        {
            channels.TemperatureChannels[i]->update();
        }
        for(int i=0; i <8; i++)
        {
            channels.PassiveChannels[i]->update();
            channels.ActiveChannels[i]->update();
        }
        raspberrypi.sendUpdate(channels);
}

void execCommand(String command)  { 

    if(command.startsWith("Update"))
    {
        updateAllChannels();
    }
    if(command.startsWith("SweepActiveChannel_"))
    {
        //channels.ActiveChannels[(int)command.charAt(18)]->startSweepIV();
        Serial.print("channels.ActiveChannels["); 
        Serial.print((int)command.charAt(18)); 
        Serial.println("]->startSweepIV();");
    }
    if(command.startsWith("StartMPPTActiveChannel_"))
    {
        //channels.ActiveChannels[(int)command.charAt(23)]->startMPPT();
        Serial.print("channels.ActiveChannels["); 
        Serial.print((int)command.charAt(23)); 
        Serial.println("]->startMPPT;");
    }
    if(command.startsWith("StopMPPTActiveChannel_"))
    {
        //channels.ActiveChannels[(int)command.charAt(22)]->stopMPPT();
        Serial.print("channels.ActiveChannels["); 
        Serial.print((int)command.charAt(22)); 
        Serial.println("]->startMPPT;");
    }
    

}