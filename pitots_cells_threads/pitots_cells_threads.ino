#include <StaticThreadController.h>
#include <ThreadController.h>
#include <Thread.h>
#include <ArduinoSTL.h>
#include "PitotBoardThread.h"
#include "CellBoardThread.h"

bool use_pitots = false;
bool use_cells = true;
bool print_pitots = false;
bool print_cells = true;
bool send_outside = false;

static const byte numChars = 32;
char receivedChars[numChars];
bool newData = false;

ThreadController controller = ThreadController();
CellBoardThread* cell_board = new CellBoardThread();
PitotBoardThread* pitot_boards[1] = {
  new PitotBoardThread(0x48),
//  new PitotBoardThread(0x49),
//  new PitotBoardThread(0x4A),
//  new PitotBoardThread(0x4B)
};


void setup(){
  Serial.begin(115200);
  initializeThreads();
}

void loop(){
  controller.run();
  printData();
  sendData();
  receiveCommands();
  interpretCommands();
}

void initializeThreads(){
  if (use_pitots){
    for (auto pitot_board : pitot_boards){
      pitot_board->setInterval(10);
      controller.add(pitot_board);
    }
  }

  if(use_cells){
    cell_board->setInterval(50);
    controller.add(cell_board);
  }
}

void printData(){
  if (print_pitots){
    for (auto pitot_board : pitot_boards){
      pitot_board->printPitots();
    }
  }
  if (print_cells){
    cell_board->printCells();
  }
  printf("\n");
}

void sendData(){
  if (send_outside){
    for (auto pitot_board : pitot_boards){
      pitot_board->sendPitots();
    }
    cell_board->sendCells();
  }
}

void receiveCommands() {
  static bool recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '!';
  char endMarker = '@';
  char rc;

  while (Serial.available() && !newData) {
    rc = Serial.read();

    if (recvInProgress) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void interpretCommands(){
  if (newData){
    if (strcmp(receivedChars, "tc") == 0) {
      cell_board->tareCells();
    }
    if (strcmp(receivedChars, "pp") == 0) {
      print_pitots = !print_pitots;
    }
    if (strcmp(receivedChars, "pc") == 0) {
      print_cells = !print_cells;
    }
    if (strcmp(receivedChars, "so") == 0) {
      send_outside = !send_outside;
    }
    newData = false;
  }
}
