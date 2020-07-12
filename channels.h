#ifndef CHANNELS_H
#define CHANNELS_H



class Channel {
    public:
        Channel(int CSPin);
        ~Channel();
        virtual void update() = 0;
    private:
        int CSPin;
};

class TemperatureChannel: public Channel {
    public:
        TemperatureChannel(int CSPin);
        ~TemperatureChannel();
        void update();
        int getTemperature();
    private:
        int temperature = 0;
};

class PassiveChannel: public Channel {
    public:
        PassiveChannel(int CSPin, int voltagePin, int switchPin);
        ~PassiveChannel();
        void update();
        int getVoltage();
        int getCurrent();
        int getPower();
        int setState(int state);
        int select();
    private:
        int voltage = 0;
        int current = 0;
        int state = 0;
        int voltagePin; 
        int switchPin; 
};

class ActiveChannel: public PassiveChannel {
    public:
        ActiveChannel(int CSPin, int voltagePin, int switchPin);
        ~ActiveChannel();
        void update();
        int sweepIV();
        int MPPT();
    private:
        int mode;
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