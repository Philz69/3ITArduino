#include "channels.h"

Channel::Channel(int CSPin) {
    this->CSPin = CSPin;
    pinMode(CSPin, OUTPUT);
    digitalWrite(CSPin, HIGH);
}

Channel::~Channel() {

}

TemperatureChannel::TemperatureChannel(int CSPin, int controlRegister):Channel(CSPin) {
    SPISetting = SPISettings(3000000, MSBFIRST, SPI_MODE1);

}

TemperatureChannel::~TemperatureChannel() {
}

int TemperatureChannel::getTemperature() {
    return temperature;
}

void TemperatureChannel::init() {
    // 0 000 111 0 100 0 1 01 0  | 0x0E 0x8A Input 1 FSR +-0.256V
    // 0 011 111 0 100 0 1 01 0 | 0x3E 0x8A Input 2 FSR +-0.256V 
    // Sets FSR to +-0.256V
    SPI.beginTransaction(SPISetting);
    digitalWrite(CSPin, LOW);
    SPI.transfer(ADS1118_INPUTREGISTER[0]);
    SPI.transfer(0x8A);
    digitalWrite(CSPin, HIGH);
    SPI.endTransaction();
}
void TemperatureChannel::update() {
    //Set input to correct channel
    SPI.beginTransaction(SPISetting);
    digitalWrite(CSPin, LOW);
    SPI.transfer(controlRegister);
    SPI.transfer(0x8A);
    digitalWrite(CSPin, HIGH);
    SPI.endTransaction();

    //Read value of previously selected input
    unsigned int msb;
    unsigned int lsb;
    SPI.beginTransaction(SPISetting);
    digitalWrite(CSPin, LOW);
    msb = SPI.transfer(0x00);
    lsb = SPI.transfer(0x00);
    digitalWrite(CSPin, HIGH);
    SPI.endTransaction();
    temperature = ( msb << 8) | lsb;
}

PassiveChannel::PassiveChannel(int CSPin, int voltagePin, int switchPin, int controlRegister):Channel(CSPin) {
   this->voltagePin = voltagePin; 
   this->switchPin = switchPin; 
   pinMode(voltagePin, INPUT);
   pinMode(switchPin, OUTPUT);
   SPISetting = SPISettings(100000, MSBFIRST, SPI_MODE0);
}

PassiveChannel::~PassiveChannel() {

}

void PassiveChannel::update() {
    digitalWrite(switchPin, LOW);
    voltage = analogRead(voltagePin) * 4.9;
    digitalWrite(switchPin, HIGH);
    SPI.beginTransaction(SPISetting);
    digitalWrite(CSPin, LOW);
    current = SPI.transfer(controlRegister) * 2000;
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

int PassiveChannel::setState(int state) {
    digitalWrite(switchPin, state);
    this->state = state;
    return 1;
}

ActiveChannel::ActiveChannel(int CSPin, int voltagePin, int switchPin, int controlRegister):PassiveChannel(CSPin, voltagePin, switchPin, controlRegister) {
}

ActiveChannel::~ActiveChannel() {    
}

void ActiveChannel::update() {
    lastVoltage = voltage;
    lastCurrent = current;
    if(simulate)
    {
        double voltageSimul = (PWM/255.0) * 5000;
        voltage = 1;
        if(PWM < 67)
        {
            current = 0.55*voltageSimul;
        }
        else
        {
            current = -( ((voltageSimul-1315)*0.1)*((voltageSimul-1315)*0.1)) + 724;
        }
        
    }
    else
    {
        PassiveChannel::update();
    }
}
int ActiveChannel::setPWM(int value) {
    lastPWM = PWM;
    analogWrite(switchPin, value);
    PWM = value;
    return 1;
}

int ActiveChannel::sweepIV() {
    for(int i = 0; i < 255; i++)
    {
        setPWM(i);
        update();
        sweepData[i % 32].voltage = voltage;
        sweepData[i % 32].current = current;
    }
    return 1;
}

void ActiveChannel::startSweepIV() {
    mode = SWEEP_MODE;
}  

void ActiveChannel::sweepIVasync() {
    if(sweepState < 255)
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

sweepResult ActiveChannel::getSweepResult(int i) {
    return sweepData[i];
}

int ActiveChannel::startMPPT() {
    this->setPWM(10);
    this->update();
    this->setPWM(PWM + 1);
    this->update();
    mode = MPPT_MODE;
}

int ActiveChannel::updateMPPT() {
    if(simulate)
    {
        /*Serial.print("Voltage: "); Serial.print(voltage); Serial.print("| Current: "); Serial.print(current); Serial.print("| Total: "); Serial.println(voltage*current);
        Serial.print("PWM: "); Serial.println(PWM);*/
    }
    if(lastVoltage*lastCurrent > voltage*current)
    {
        this->setPWM(PWM + (lastPWM-PWM));
    }
    else
    {
        this->setPWM(PWM - (lastPWM-PWM));
    }
    this->update();
    return 1;
}

int ActiveChannel::stopMPPT() {
    mode = STATIC_MODE;
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
        this->ActiveChannels[i] = new ActiveChannel(CSPins[i/4+10], AnalogPins[i], PWMPins[i], ADC124S_INPUTREGISTER[i%4]);
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