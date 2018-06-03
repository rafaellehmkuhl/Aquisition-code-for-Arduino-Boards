#pragma once
#include <Adafruit_ADS1015.h>
#include <Wire.h>
#include "Pitot.h"

class PitotBoardThread: public Thread
{
  public:
    std::vector<Pitot> pitots;

    PitotBoardThread(byte adress) :
      ads(adress),
      pitots{
        Pitot(0, "pitot0", ads),
        Pitot(1, "pitot1", ads),
        Pitot(2, "pitot2", ads),
        Pitot(3, "pitot3", ads)
      }
      {
        initialize();
      }

    void initialize(){
      ads.begin();
    }

    void run(){
      for (Pitot& pitot : pitots){
        pitot.updateVoltage();
      }

      runned();
    }

    void printPitots(){

      for (Pitot& pitot : pitots){
        printf("%f\t", pitot.Voltage);
      }
    }

    void sendPitots(){
      printf("!");
      for (Pitot& pitot : pitots){
        printf("%s=%f;", pitot.apelido.c_str(), pitot.Voltage);
      }
      printf("@\n");
    }

  private:
    Adafruit_ADS1015 ads;
};