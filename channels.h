#ifndef CHANNELS_H
#define CHANNELS_H

#include <SPI.h>

const int ADC124S_INPUTREGISTER[4] = {0x00,0x08,0x10,0x18};
const int ADS1118_INPUTREGISTER[2] = {0x0E, 0x3E};
const int AnalogPins[16] = {54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69};
const int SwitchPins[8] = {42,43,44,45,46,47,48,49};
const int CSPins[13] = {22,23,24,25,26,27,28,29,30,31,32,33,34};
const int PWMPins[9] = {2,3,5,6,7,8,9,10,11};

struct sweepResult {
    int voltage = 0;
    int current = 0;
};

class Channel {
    public:
        Channel(int CSPin);
        ~Channel();
        virtual void update() = 0;
    protected:
        SPISettings SPISetting;
        int controlRegister;
        int CSPin;
};

class TemperatureChannel: public Channel {
    public:
        TemperatureChannel(int CSPin, int controlRegister);
        ~TemperatureChannel();
        void init();
        void update();
        int getTemperature();
    private:
        int temperature = 0;
};

class PassiveChannel: public Channel {
    public:
        PassiveChannel(int CSPin, int voltagePin, int switchPin, int controlRegister);
        ~PassiveChannel();
        void update();
        int getVoltage();
        int getCurrent();
        int getPower();
        int getCSPin();
        int setState(int state);
        int select();
    protected:
        int voltage = 0;
        int current = 0;
        int state = 0;
        int voltagePin; 
        int switchPin; 
};

class ActiveChannel: public PassiveChannel {
    public:
        ActiveChannel(int CSPin, int voltagePin, int switchPin, int controlRegister);
        ~ActiveChannel();
        void update();
        int sweepIV();
        int sweepIVasync();
        int startMPPT();
        int updateMPPT();
        int stopMPPT();
        int setPWM(int value);
        sweepResult getSweepResult(int i);
    protected:
        sweepResult sweepData[255];
        int PWM = 0;
        int sweepState = 0;
        int lastPWM = 0;
        int lastVoltage;
        int lastCurrent;
        int mode;
        bool simulate = true;
};

class Channels {
    public:
        Channels();
        ~Channels();
        TemperatureChannel* TemperatureChannels[16];
        PassiveChannel* PassiveChannels[8];
        ActiveChannel* ActiveChannels[8];
};

#endif