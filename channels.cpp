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
    ads1118 = new ADS1118(CSPin);
    this->controlRegister = controlRegister;
    if(controlRegister == ADS1118_INPUTREGISTER[0]) {
        delayMesurement = 100;
        delayChannelChange = 0;
    }
    else
    {
        delayChannelChange = 200;
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
    ads1118->begin();
    ads1118->setFullScaleRange(ads1118->FSR_0256);
    ads1118->setContinuousMode();
    lastChannelChange = millis();
    /*SPI.beginTransaction(SPISetting);
    digitalWrite(CSPin, LOW);
    SPI.transfer16(controlRegister);
    //SPI.transfer16(0x2170);
    //SPI.transfer(ADS1118_INPUTREGISTER[0]);
    //SPI.transfer(0x8A);
    digitalWrite(CSPin, HIGH);
    SPI.endTransaction();*/
}
void TemperatureChannel::update() {
    //Set input to correct channel
    if(mode == DONE){
        mode = GETTING_TEMP;
        lastChannelChange = millis();
    }
    if(((millis() - lastChannelChange) > delayChannelChange) && mode == GETTING_TEMP)
    {
        /*Serial.print("Entering channel switch: lastChange:");
        Serial.print(millis() - lastChannelChange);
        Serial.print("| mode: ");
        Serial.println(mode);*/
        SPI.beginTransaction(SPISetting);
        digitalWrite(CSPin, LOW);
        SPI.transfer16(controlRegister);
        //SPI.transfer16(0x2170);
        //SPI.transfer(controlRegister);
        //SPI.transfer(0x8A);
        digitalWrite(CSPin, HIGH);
        SPI.endTransaction();
        //ads1118->setInputSelected(controlRegister);
        mode = DELAYING;
        lastChannelChange = millis();
    }
    if(((millis() - lastChannelChange) > delayMesurement) && mode == DELAYING) {
        //Serial.println("Entering measurement");
        SPI.beginTransaction(SPISetting);
        digitalWrite(CSPin, LOW);
        double result = SPI.transfer16(0x0000);
        if(result < 65500) {
            temperature = result;
        }
        digitalWrite(CSPin, HIGH);
        SPI.endTransaction();
        //ads1118->getTemperature();
        mode = DONE;
    }
    /*SPI.beginTransaction(SPISetting);
    digitalWrite(CSPin, LOW);
    SPI.transfer16(controlRegister);
    //SPI.transfer16(0x2170);
    //SPI.transfer(controlRegister);
    //SPI.transfer(0x8A);
    digitalWrite(CSPin, HIGH);
    SPI.endTransaction();

    delay(100);
    //Read value of previously selected input
    SPI.beginTransaction(SPISetting);
    digitalWrite(CSPin, LOW);
    //msb = SPI.transfer(0x00);
    //lsb = SPI.transfer(0x00);
    temperature = SPI.transfer16(0x0000);
    //temperature = SPI.transfer16(0x000);
    //Serial.print("MSB:"); Serial.println(msb);
    //Serial.print("LSB:"); Serial.println(lsb);
    digitalWrite(CSPin, HIGH);
    SPI.endTransaction();
    //temperature = ( msb << 8) | lsb;*/
}

int TemperatureChannel::getMode() {
    return mode;
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
        voltage = analogRead(voltagePin) * 4.9;
        SPI.beginTransaction(SPISetting);
        digitalWrite(CSPin, LOW);
        current = SPI.transfer(controlRegister) * 2000;
        digitalWrite(CSPin, HIGH);
        SPI.endTransaction();
    }
}
int ActiveChannel::getPWM() {
    return PWM;
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