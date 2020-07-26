#ifndef MASTER_H
#define MASTEr_H

#include "channels.h"
#include <ArduinoJson.h>

class Master {
    public:
        Master();
        String getCommand();
        void sendUpdate(Channels &channels);
        void sendPartialSweepData(sweepResult sweepResult[32]);
};

#endif