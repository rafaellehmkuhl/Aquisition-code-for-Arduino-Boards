#pragma once

class Cell: public HX711_ADC
{
    public:
        String apelido;
        float force;
        float cal_factor;
        int is_ready = 0;
        Cell(int doutPin, int sckPin, float cal_factor, String apelido) :
            HX711_ADC(doutPin, sckPin)
        {}

        void updateForce(){
            force = 9.81 * HX711_ADC::getData();
        }
};