#include "channels.h"

Channel::Channel(int CSPin, unsigned int controlRegister) {
    this->controlRegister = controlRegister;
    this->CSPin = CSPin;
    pinMode(CSPin, OUTPUT);
    digitalWrite(CSPin, HIGH);
}

Channel::~Channel() {

}

TemperatureChannel::TemperatureChannel(int CSPin, unsigned int controlRegister):Channel(CSPin, controlRegister) {
    SPISetting = SPISettings(3000000, MSBFIRST, SPI_MODE1);
    if(controlRegister == ADS1118_INPUTREGISTER[0]) {
        delayMesurement = 100;
        delayChannelChange = 0;
    }
    else
    {
        delayChannelChange = 350;
        delayMesurement = 100;
    }
    
}

TemperatureChannel::~TemperatureChannel() {
}

double TemperatureChannel::getTemperature() {
    return temperature;
}

void TemperatureChannel::init() {
    // 0  000111 0 100 0 0 01 0  | 0x0E 0x8A Input 1 FSR +-0.256V
    // 0 011 111 0 100 0 0 01 0 | 0x3E 0x8A Input 2 FSR +-0.256V 
    // Sets FSR to +-0.256V
    SPI.beginTransaction(SPISetting);
    digitalWrite(CSPin, LOW);
    SPI.transfer16(controlRegister);
    digitalWrite(CSPin, HIGH);
    SPI.endTransaction();
    lastChannelChange = millis();
}
void TemperatureChannel::update() {
    if(mode == DONE){
        mode = GETTING_TEMP;
        lastChannelChange = millis();
    }
    if(((millis() - lastChannelChange) > delayChannelChange) && mode == GETTING_TEMP)
    {
        SPI.beginTransaction(SPISetting);
        digitalWrite(CSPin, LOW);
        SPI.transfer16(controlRegister);
        digitalWrite(CSPin, HIGH);
        SPI.endTransaction();
        mode = DELAYING;
        lastChannelChange = millis();
    }
    if(((millis() - lastChannelChange) > delayMesurement) && mode == DELAYING) {
        SPI.beginTransaction(SPISetting);
        digitalWrite(CSPin, LOW);
        unsigned int result = SPI.transfer16(controlRegister | 0x10); //Reads millivolts of selected channel and selects internal temperature sensor for the next reading
        digitalWrite(CSPin, HIGH);
        SPI.endTransaction();
        if(result>=0x8000) //Millivolts are negative
        {
            result=((~result)+1);
            millivolts = (result*0.256/32768)*-1000;
        }
        else //Millivolts are positive
        {
            millivolts = result*0.256/32768*1000;
        }
        mode = GETTING_AMBIENT;
        lastChannelChange = millis();
    }
    if(((millis() - lastChannelChange) > delayMesurement) && mode == GETTING_AMBIENT) {
        SPI.beginTransaction(SPISetting);
        digitalWrite(CSPin, LOW);
        unsigned int ambientResult = SPI.transfer16(0x0000); //Reads the ambient temperature
        digitalWrite(CSPin, HIGH);
        SPI.endTransaction();

        //The result is 14 bit, left justified but our value contains 16 bits. Shift 2 bit to the right to compensate
        ambientResult = ambientResult>>2; 
        if(ambientResult >= 0x2000) //Ambient temp is negative
        {
           ambientTemp = ambientResult * -0.03125;    
        }
        else //Ambient temp is positive
        {
            ambientTemp = ambientResult * 0.03125;    
        }

        /*Serial.println("millivolts: ");
        Serial.println(millivolts);*/
        temperature = thermocoupleConvertWithCJCompensation(millivolts * 1000, ambientTemp * 1000) / 1000;
        if(temperature > 1200)
        {
            temperature = 0;
        }
        else if(temperature < -200)
        {
            temperature = 0;
        }
        mode = DONE;
    }
}

int TemperatureChannel::getMode() {
    return mode;
}


PassiveChannel::PassiveChannel(int CSPin, int voltagePin, int switchPin, unsigned int controlRegister):Channel(CSPin, controlRegister) {
   this->voltagePin = voltagePin; 
   this->switchPin = switchPin; 
   pinMode(voltagePin, INPUT);
   pinMode(switchPin, OUTPUT);
   SPISetting = SPISettings(3000000, MSBFIRST, SPI_MODE3);
}

PassiveChannel::~PassiveChannel() {

}

void PassiveChannel::update() {
    digitalWrite(switchPin, LOW);
    voltage = analogRead(voltagePin) * 4.9 / 0.7;
    digitalWrite(switchPin, HIGH);
    //delay(100);
    SPI.beginTransaction(SPISetting);
    digitalWrite(CSPin, LOW);
    current = SPI.transfer16(controlRegister) * 5000.0/4096.0 / 2;
    digitalWrite(CSPin, HIGH); 
    SPI.endTransaction();
    digitalWrite(switchPin, LOW);
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

int PassiveChannel::getCSPin() {
    return CSPin;
}

int PassiveChannel::getSwitchPin() {
    return switchPin;
}

int PassiveChannel::setState(int state) {
    digitalWrite(switchPin, state);
    this->state = state;
    return 1;
}

ActiveChannel::ActiveChannel(int CSPin, int voltagePin, int switchPin, unsigned int controlRegister):PassiveChannel(CSPin, voltagePin, switchPin, controlRegister) {
}

ActiveChannel::~ActiveChannel() {    
}

void ActiveChannel::update() {
    lastVoltage = voltage;
    lastCurrent = current;
    if(simulate) //if you want to test the MPPT algorithm without a solar panel. Mimics the power curve of a real solar panel
    {
        double voltageSimul = (PWM/255.0) * 5000;
        voltage = 1;
        if(PWM < 67)
        {
            current = 0.55*voltageSimul;
        }
        else if(PWM < 81)
        {
            current = -( ((voltageSimul-1315)*0.1)*((voltageSimul-1315)*0.1)) + 724;
        }
        else
        {
            current = 0;
        }
        
    }
    else
    {
            if(mode == STATIC_MODE)
            {
                this->setPWM(0);
            }
            voltage = analogRead(voltagePin) * 4.9 / 0.7;
            if(mode == STATIC_MODE)
            {
                this->setPWM(255);
            }
            SPI.beginTransaction(SPISetting);
            digitalWrite(CSPin, LOW);
            current = SPI.transfer16(controlRegister) * 5000.0/4096.0 / 2;
            current = SPI.transfer16(controlRegister) * 5000.0/4096.0 / 2;
            digitalWrite(CSPin, HIGH); 
            SPI.endTransaction();
            if(mode == STATIC_MODE)
            {
                this->setPWM(0);
            }
    }
        if(voltage == 0)
        {
            voltage = 1;
        }
        if(current == 0)
        {
            current = 1;
        }
}
int ActiveChannel::getPWM() {
    return PWM;
}
int ActiveChannel::setPWM(int value) {
    lastPWM = PWM;
    if(value > 255)
    {
        value = 255;
    }
    if(value < 0)
    {
        value = 255;
    }
    analogWrite(switchPin, value);
    PWM = value;
    return 1;
}

void ActiveChannel::startSweepIV() {
    mode = SWEEP_MODE;
    sweepState = 1;
    setPWM(0);
    update();
}  

void ActiveChannel::sweepIVasync() {
    if(sweepState <= 255)
    {
        setPWM(sweepState);
        sweepState += 1;
        update();
        sweepData[sweepState % 32].voltage = voltage;
        sweepData[sweepState % 32].current = current;
    }
    else
    {
        sweepState = 0;
        mode = STATIC_MODE;
    }
}

sweepResult* ActiveChannel::getSweepResult() {
    return sweepData;
}

int ActiveChannel::startMPPT() {
    this->setPWM(10);
    this->update();
    this->setPWM(PWM + 1);
    this->update();
    mode = MPPT_MODE;
}

int ActiveChannel::updateMPPT() {
    this->update(SWEEP_NMB_POINTS_AVG);
    long power = voltage * 1L * current;
    long lastPower = lastVoltage* 1L * lastCurrent;
    if(lastPower > power)
    {
        if(lastPWM > PWM)
        {
            this->setPWM(PWM + MPPT_PWM_OFFSET);
        }
        else
        {
            this->setPWM(PWM - MPPT_PWM_OFFSET);
        }
    }
    else
    {
        if(lastPWM > PWM)
        {
            this->setPWM(PWM - MPPT_PWM_OFFSET);
        }
        else
        {
            this->setPWM(PWM + MPPT_PWM_OFFSET);
        }
    }
    return 1;
}

int ActiveChannel::stopMPPT() {
    mode = STATIC_MODE;
    setPWM(0);
    return 1;
}

int ActiveChannel::getMode() {
    return mode;
}

Channels::Channels() {
    for(int i = 0; i < 8; i++)
    {
        this->TemperatureChannels[i*2] = new TemperatureChannel(CSPins[i], ADS1118_INPUTREGISTER[0]);
        this->TemperatureChannels[i*2 + 1] = new TemperatureChannel(CSPins[i], ADS1118_INPUTREGISTER[1]);
    }
    for(int i = 0; i < 8; i++)
    {
        this->PassiveChannels[i] = new PassiveChannel(CSPins[i/4+8], AnalogPins[i], SwitchPins[i], ADC124S_INPUTREGISTER[i%4]);
    }
    for(int i = 0; i < 8; i++)
    {
        this->ActiveChannels[i] = new ActiveChannel(CSPins[i/4+10], AnalogPins[i+8], PWMPins[i], ADC124S_INPUTREGISTER[i%4]);
    }
    this->ActiveChannels[8] = new ActiveChannel100W(CSPins[12], PWMPins[8], ADC124S_INPUTREGISTER[3], ADC124S_INPUTREGISTER[2], ADC124S_INPUTREGISTER[1]);
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
   delete(this->ActiveChannels[8]);
}


ActiveChannel100W::ActiveChannel100W(int CSPin, int switchPin, unsigned int VSenseNControlRegister, unsigned int VSensePControlRegister, unsigned int CurrentControlRegister):ActiveChannel(CSPin, 0, switchPin, 0){
        this->VSenseNControlRegister = VSenseNControlRegister;
        this->VSensePControlRegister = VSensePControlRegister;
        this->CurrentControlRegister = CurrentControlRegister;
}

void ActiveChannel100W::update() 
{
        lastVoltage = voltage;
        lastCurrent = current;

        if(mode == STATIC_MODE) //Si il n'est pas en mppt ni en Sweep
        {
            setPWM(0);
        }
        SPI.beginTransaction(SPISetting);
        digitalWrite(CSPin, LOW);
        unsigned int VsensePVoltage = SPI.transfer16(VSensePControlRegister) * 5000.0/4096.0 / 0.3; //Nmb of lsb * size of lsb / voltage divider ratio
        VsensePVoltage = SPI.transfer16(VSensePControlRegister) * 5000.0/4096.0 / 0.3; //Nmb of lsb * size of lsb / voltage divider ratio
        VsensePVoltage = SPI.transfer16(VSensePControlRegister) * 5000.0/4096.0 / 0.3; //Nmb of lsb * size of lsb / voltage divider ratio
        digitalWrite(CSPin, HIGH);
        SPI.endTransaction();


        SPI.beginTransaction(SPISetting);
        digitalWrite(CSPin, LOW);
        unsigned int VsenseNVoltage = SPI.transfer16(VSenseNControlRegister) * 5000.0/4096.0; //Nmb of lsb * size of lsb
        VsenseNVoltage = SPI.transfer16(VSenseNControlRegister) * 5000.0/4096.0; //Nmb of lsb * size of lsb
        VsenseNVoltage = SPI.transfer16(VSenseNControlRegister) * 5000.0/4096.0; //Nmb of lsb * size of lsb
        digitalWrite(CSPin, HIGH);
        SPI.endTransaction();

        if(mode == STATIC_MODE)
        {
            setPWM(255);
        }

        SPI.beginTransaction(SPISetting);
        digitalWrite(CSPin, LOW);
        current = SPI.transfer16(CurrentControlRegister) * 5000.0/4096.0 / 0.2;
        current = SPI.transfer16(CurrentControlRegister) * 5000.0/4096.0 / 0.2;
        current = SPI.transfer16(CurrentControlRegister) * 5000.0/4096.0 / 0.2;
        digitalWrite(CSPin, HIGH);
        SPI.endTransaction();
        //SPI.beginTransaction(SPISetting);
        //digitalWrite(CSPin, LOW);
        //current = SPI.transfer16(CurrentControlRegister) * 5000.0/4096.0 / 0.2;
        //current = SPI.transfer16(CurrentControlRegister) * 5000.0/4096.0 / 0.2;
        //digitalWrite(CSPin, HIGH);
        //SPI.endTransaction();

        voltage = VsensePVoltage - VsenseNVoltage;

        if(mode == STATIC_MODE)
        {
            setPWM(0);
        }

        //Serial.print("Vsense+");
        //Serial.print(VsensePVoltage);
        //Serial.print("Vsense-");
        //Serial.println(VsenseNVoltage);
        if(current == 0)
        {
            current = 1;
        }
        if(voltage == 0)
        {
            voltage = 1;
        }
}
void ActiveChannel::update(int nmbAverage)
{
    int initialVoltage = voltage;
    int initialCurrent = current;
    float averageCurrent = 0;
    float averageVoltage = 0;

    for(int i = 0; i < nmbAverage; i++)
    {
        update();
        averageCurrent += current / nmbAverage;
        averageVoltage += voltage / nmbAverage;
    }
    current = averageCurrent;
    voltage = averageVoltage;
    lastVoltage = initialVoltage;
    lastCurrent = initialCurrent;
}