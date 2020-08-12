#ifndef CHANNELS_H
#define CHANNELS_H

#include <SPI.h>
#include "ADS1118.h"

const int ADC124S_INPUTREGISTER[4] = {0x00,0x08,0x10,0x18};
const int ADS1118_INPUTS[2] = {0b000, 0b011};
const unsigned int ADS1118_INPUTREGISTER[2] = {0xE82, 0x3E82};//{0x8f82, 0xBF82};;
const int AnalogPins[16] = {54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69};
const int SwitchPins[8] = {42,43,44,45,46,47,48,49};
const int CSPins[13] = {22,23,24,25,26,27,28,29,30,31,32,33,34};
const int PWMPins[9] = {2,3,5,6,7,8,9,10,11};

const int STATIC_MODE = 0;
const int MPPT_MODE = 1;
const int SWEEP_MODE = 2;

const int GETTING_TEMP = 0;
const int DELAYING = 1;
const int DONE = 2;

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
        double getTemperature();
        int getMode();
    private:
        int mode = DONE;
        int delayMesurement = 50;
        int delayChannelChange;
        long lastChannelChange;
        long lastChannelMeasurement;
        ADS1118* ads1118;
        double temperature = 0;
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
        void startSweepIV();
        void sweepIVasync();
        int startMPPT();
        int updateMPPT();
        int stopMPPT();
        int setPWM(int value);
        int getPWM();
        int getMode();
        sweepResult* getSweepResult();
    protected:
        sweepResult sweepData[32];
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