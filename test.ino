#include <arduino.h>
#include "channels.h"
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

//Channels channels;

void setup()
{
    Serial.begin(9600);
    Channels channelsssss;
}

void loop()
{
    //channels.PassiveChannels[0]->update();
    //voltage = channels.PassiveChannels[0]->getVoltage();
    //Serial.println(voltage);
    delay(1000);
}