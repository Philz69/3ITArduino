#include <arduino.h>
#include "channels.h"
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


//Channels channels;

bool sweetOnce = true;
char serialInChars[255];

void setup()
{
    digitalWrite(LED_BUILTIN,LOW);
    Serial.begin(115200);
    Serial.println("test1");
    SPI.begin();

    Channels channels;
    Serial.println("test2");
    for(int i = 0; i < 8; i++)
    {
        ////channels.TemperatureChannels[i]->init();
    }
    setPWMScaler(1);
    pinMode(ledPin, OUTPUT);
    //channels.ActiveChannels[0]->startMPPT();
}

void loop() { 
    /*for(int i=0; i < 8; i++)
    {
        channels.TemperatureChannels[i]->update();
    }
    for(int i=0; i < 16; i++)
    {
        channels.PassiveChannels[i]->update();
        channels.ActiveChannels[i]->update();
    }*/
    //channels.ActiveChannels[0]->updateMPPT();

    //sweepActiveChannels();
    /*for(int i = 0; i < 8; i++)
    {
        sweepResult sweepData[255] = channels.ActiveChannels[i].getSweepResult();
        for(int j = 0; j < 255; j++)
        {
            Serial.print(sweepData[j].voltage
        }
    }*/
    digitalWrite(LED_BUILTIN,HIGH);
    Serial.println("looopin");
    delay(1000);
    digitalWrite(LED_BUILTIN,LOW);
    delay(1000);
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

void sweepActiveChannels() {
    bool sweepStatus[8] = {false};
    while(sweepStatus[8])
    {
        for(int i = 0; i < 8; i++)
        {
            if(!sweepStatus[i])
            {
                //sweepStatus[i] = channels.ActiveChannels[i]->sweepIVasync();
            }
        }
    }   
}