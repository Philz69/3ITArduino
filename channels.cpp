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
