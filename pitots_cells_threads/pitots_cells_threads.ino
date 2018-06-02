#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

#include <ArduinoSTL.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

#include "PitotThread.h"
#include "CellsThread.h"
#include "BancadaFunctions.h"

bool use_pitots = false;
bool print_pitots = false;
bool send_pitots_via_protocol = false;
int numPitotBoards = 1;

bool use_cells = true;
bool print_cells = true;
bool send_cells_via_protocol = false;

bool send_outside = false;

BancadaFunctions bancada;

//ADS1015 constructor
Adafruit_ADS1015 ads0(0x48);
Adafruit_ADS1015 ads1(0x49);
Adafruit_ADS1015 ads2(0x4A);
Adafruit_ADS1015 ads3(0x4B);

ThreadController controller = ThreadController();

std::vector<PitotThread> pitots = {
  PitotThread(0, "pitot0", ads0),
  PitotThread(1, "pitot1", ads0),
  PitotThread(2, "pitot2", ads0),
  PitotThread(3, "pitot3", ads0),
  PitotThread(0, "pitot4", ads1),
  PitotThread(1, "pitot5", ads1),
  PitotThread(2, "pitot6", ads1),
  PitotThread(3, "pitot7", ads1),
  PitotThread(0, "pitot8", ads2),
  PitotThread(1, "pitot9", ads2),
  PitotThread(2, "pitot10", ads2),
  PitotThread(3, "pitot11", ads2),
  PitotThread(0, "pitot12", ads3),
  PitotThread(1, "pitot13", ads3),
  PitotThread(2, "pitot14", ads3),
  PitotThread(3, "pitot15", ads3)
};

CellsThread celulas_bancada = CellsThread();

void setup(){
  Serial.begin(115200);

  if (use_pitots){
    for (PitotThread& pitot : pitots){
      pitot.setInterval(1);
      controller.add(&pitot);
    }

    ads0.begin();
    ads1.begin();
    ads2.begin();
    ads3.begin();
  }

  if(use_cells){
    celulas_bancada.initializeCells();
    celulas_bancada.setInterval(1);
    controller.add(&celulas_bancada);
  }
}

void loop(){

  controller.run();

  if (print_pitots){
    bancada.printPitots(numPitotBoards, pitots);
  }

  if (print_cells){
    bancada.printCells(celulas_bancada);
  }

  printf("\n");

  if (send_outside){
    bancada.sendDataViaProtocol(send_cells_via_protocol, send_pitots_via_protocol, numPitotBoards, pitots, celulas_bancada);
  }

  bancada.receiveCommands();
  bancada.interpretCommands(celulas_bancada, print_pitots, print_cells, send_outside);
}