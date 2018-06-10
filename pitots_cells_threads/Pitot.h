#pragma once

class Pitot
{
    public:
        String apelido;
        float Voltage = 0.0;
        int16_t adc = 0;
        int adc_port;
        Adafruit_ADS1015 &ads;
        Pitot(int adc_port, String apelido, Adafruit_ADS1015 &ads){};

        void updateVoltage(){
            adc = ads.readADC_SingleEnded(adc_port);
            Voltage = adc * 0.1875;
        }
};