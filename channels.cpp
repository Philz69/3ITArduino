#include "channels.h"

Channel::Channel(int CSPin) {
    this->CSPin = CSPin;
    pinMode(CSPin, OUTPUT);
}

Channel::~Channel() {

}

TemperatureChannel::TemperatureChannel(int CSPin):Channel(CSPin) {
}

TemperatureChannel::~TemperatureChannel() {
}

void TemperatureChannel::update() {

}

PassiveChannel::PassiveChannel(int CSPin, int voltagePin, int switchPin):Channel(CSPin) {
   this->voltagePin = voltagePin; 
   this->switchPin = switchPin; 
   pinMode(voltagePin, INPUT);
   pinMode(switchPin, OUTPUT);
}

PassiveChannel::~PassiveChannel() {

}


void PassiveChannel::update() {
    voltage = analogRead(voltagePin) * 4.9;
}
int PassiveChannel::getVoltage() {
    return voltage;
}
int PassiveChannel::getCurrent() {
    return current;
}
int PassiveChannel::getPower() {
    return voltage*current;
}
int PassiveChannel::setState(int state) {
    return 1;
}
int PassiveChannel::select() {
    return 1;
}

ActiveChannel::ActiveChannel(int CSPin, int voltagePin, int switchPin):PassiveChannel(CSPin, voltagePin, switchPin) {
}

ActiveChannel::~ActiveChannel() {    
}

void ActiveChannel::update() {
}
int ActiveChannel::sweepIV() {
    return 1;
}
int ActiveChannel::MPPT() {
    return 1;
}

Channels::Channels() {
    int firstAnalogPin = 54;
    int firstCSPin = 22;
    int firstSwitchPin = 46;
    for(int i = 0; i < 16; i++)
    {
        Serial.print("TemperatureChannel ["); Serial.print(i); Serial.print("] CSPin = "); Serial.print(firstCSPin);  Serial.println();
        this->TemperatureChannels[i] = new TemperatureChannel(firstCSPin);
        if(i % 2 != 0)
        firstCSPin += 1;
    }
    for(int i = 0; i < 8; i++)
    {
        Serial.print("PassiveChannel ["); Serial.print(i); Serial.print("]");
        Serial.print(" CSPin = "); Serial.print(firstCSPin); 
        Serial.print(" firstAnalogPin ="); Serial.print(firstAnalogPin);
        Serial.print(" firstSwitchPin ="); Serial.print(firstSwitchPin);
        Serial.println();
        this->PassiveChannels[i] = new PassiveChannel(firstCSPin, firstAnalogPin, firstSwitchPin + i);
        if(i == 3 | i == 7) {
            firstCSPin += 1;
        }
        firstAnalogPin += 1;
        firstSwitchPin += 1;
    }
    int firstPWMPin = 2;
    for(int i = 0; i < 8; i++)
    {
        Serial.print("ActiveChannel ["); Serial.print(i); Serial.print("]");
        Serial.print(" CSPin = "); Serial.print(firstCSPin); 
        Serial.print(" firstAnalogPin ="); Serial.print(firstAnalogPin);
        Serial.print(" firstPWMPin ="); Serial.print(firstPWMPin);
        Serial.println();
        this->ActiveChannels[i] = new ActiveChannel(firstCSPin, firstAnalogPin, firstPWMPin);
        if(i == 2) {
            firstPWMPin += 2;
        }
        else
        {
            firstPWMPin += 1;
        }
        
        if(i == 3 | i == 7) {
            firstCSPin += 1;
        }
        firstAnalogPin += 1;
    }

}

Channels::~Channels()
{
   for(int i = 0; i < 16; i++)
   {
       if(i < 8)
       {
        delete(this->PassiveChannels[i]);
        delete(this->ActiveChannels[i]);
       }
       delete(this->TemperatureChannels[i]);
   }
}