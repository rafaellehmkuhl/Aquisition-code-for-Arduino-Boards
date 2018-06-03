#include <Adafruit_ADS1015.h>
#include <Wire.h>
#include "Pitot.h"
#include "BancadaFunctions.h"
#pragma once

class PitotBoardThread: public Thread
{
  public:
    std::vector<Pitot> pitots;

    PitotBoardThread(byte adress) :
      ads(adress),
      pitot0(0, "pitot0", ads),
      pitot1(1, "pitot1", ads),
      pitot2(2, "pitot2", ads),
      pitot3(3, "pitot3", ads)
      {
        pitots.push_back(pitot0);
        pitots.push_back(pitot1);
        pitots.push_back(pitot2);
        pitots.push_back(pitot3);
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
    Pitot pitot0;
    Pitot pitot1;
    Pitot pitot2;
    Pitot pitot3;
};