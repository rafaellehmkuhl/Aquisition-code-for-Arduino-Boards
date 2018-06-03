#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

#include <ArduinoSTL.h>

#include "PitotBoardThread.h"
#include "CellsThread.h"
#include "BancadaFunctions.h"

bool use_pitots = false;
bool print_pitots = false;

bool use_cells = true;
bool print_cells = true;

bool send_outside = false;

BancadaFunctions bancada;

ThreadController controller = ThreadController();

std::vector<PitotBoardThread> pitot_boards = {
  PitotBoardThread(0x48),
  PitotBoardThread(0x49),
  PitotBoardThread(0x4A),
  PitotBoardThread(0x4B)
};

CellsThread celulas_bancada = CellsThread();

void setup(){
  Serial.begin(115200);

  if (use_pitots){
    for (PitotBoardThread& pitot_board : pitot_boards){
      pitot_board.setInterval(1);
      controller.add(&pitot_board);
      pitot_board.initialize();
    }
  }

  if(use_cells){
    celulas_bancada.initialize();
    celulas_bancada.setInterval(1);
    controller.add(&celulas_bancada);
  }
}

void loop(){

  controller.run();

  if (print_pitots){
    for (PitotBoardThread& pitot_board : pitot_boards){
      pitot_board.printPitots();
    }
  }

  if (print_cells){
    celulas_bancada.printCells();
  }

  printf("\n");

  if (send_outside){
    for (PitotBoardThread& pitot_board : pitot_boards){
      pitot_board.sendPitots();
    }
    celulas_bancada.sendCells();
  }

  bancada.receiveCommands();
  // bancada.interpretCommands(celulas_bancada, print_pitots, print_cells, send_outside);
}