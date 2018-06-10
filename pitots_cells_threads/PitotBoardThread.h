#pragma once
#include <Adafruit_ADS1015.h>
#include <Wire.h>
#include "Pitot.h"

class PitotBoardThread: public Thread
{
  public:
    Pitot* pitots[4];

    PitotBoardThread(byte adress) :
      ads(adress),
      pitots{
        new Pitot(0, "P00", ads),
        new Pitot(1, "P01", ads),
        new Pitot(2, "P02", ads),
        new Pitot(3, "P03", ads)
      }
      {
        initialize();
      }

    void initialize(){
      ads.begin();
    }

    void run(){
      for (auto pitot : pitots){
        pitot->updateVoltage();
      }

      runned();
    }

    void printPitots(){

      for (auto pitot : pitots){
        printf("%f\t", pitot->Voltage);
      }
    }

    void sendPitots(){
      printf("!");
      for (auto pitot : pitots){
        printf("%s=%f;", pitot->apelido.c_str(), pitot->Voltage);
      }
      printf("@\n");
    }

  private:
    Adafruit_ADS1015 ads;
};
