#ifndef MASTER_H
#define MASTEr_H

#include "channels.h"
#include <ArduinoJson.h>

class Master {
    public:
        Master();
        String getCommand();
        void sendData(Channels channels);
    private:
        long baudRate = 115200;
};

#endif