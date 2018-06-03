#include <StaticThreadController.h>
#include <ThreadController.h>
#include <Thread.h>
#include <ArduinoSTL.h>
#include "PitotBoardThread.h"
#include "CellBoardThread.h"
#include "BancadaFunctions.h"

bool use_pitots = false;
bool use_cells = true;
bool print_pitots = false;
bool print_cells = true;
bool send_outside = false;

BancadaFunctions bancada;

ThreadController controller = ThreadController();
CellBoardThread cell_board = CellBoardThread();
std::vector<PitotBoardThread> pitot_boards = {
  PitotBoardThread(0x48),
  PitotBoardThread(0x49),
  PitotBoardThread(0x4A),
  PitotBoardThread(0x4B)
};


void setup(){
  Serial.begin(115200);
  initializeThreads();
}

void loop(){
  controller.run();
  printData();
  sendData();
  bancada.receiveCommands();
  bancada.interpretCommands(cell_board, print_pitots, print_cells, send_outside);
}

void initializeThreads(){
  if (use_pitots){
    for (PitotBoardThread& pitot_board : pitot_boards){
      pitot_board.setInterval(1);
      controller.add(&pitot_board);
    }
  }

  if(use_cells){
    cell_board.setInterval(1);
    controller.add(&cell_board);
  }
}

void printData(){
  if (print_pitots){
    for (PitotBoardThread& pitot_board : pitot_boards){
      pitot_board.printPitots();
    }
  }
  if (print_cells){
    cell_board.printCells();
  }
  printf("\n");
}

void sendData(){
  if (send_outside){
    for (PitotBoardThread& pitot_board : pitot_boards){
      pitot_board.sendPitots();
    }
    cell_board.sendCells();
  }
}