#pragma once

class Pitot
{
    public:
        float Voltage = 0.0;
        int16_t adc = 0;
        int adc_port;
        String apelido;
        Adafruit_ADS1015 ads;
        
        Pitot(int _adc_port, String _apelido, Adafruit_ADS1015 &_ads) :
          adc_port(_adc_port),
          apelido(_apelido),
          ads(_ads)
          {}

        void updateVoltage(){
            adc = ads.readADC_SingleEnded(adc_port);
            Voltage = adc * 0.1875;
        }
};
