#pragma once
#include <HX711_ADC.h>
#include "Cell.h"

class CellBoardThread: public Thread
{
public:
  float calibrationTime = 5000; // tare preciscion can be improved by adding a few seconds of stabilising time
  std::vector<Cell*> cells;

  CellBoardThread() :
    cells{
      new Cell(3, 4, 213.0, "CH"),
      new Cell(5, 6, 213.0, "CFD"),
      new Cell(7, 8, 213.0, "CFE"),
      new Cell(9, 10, 213.0, "CTD"),
      new Cell(11, 12, 213.0, "CTE")
    }
    {
      initialize();
    }

  void initialize(){

    for (auto cell : cells){
      cell->begin();
    }

    while (_cells_ready < 5){
      _cells_ready = 0;
      for (auto cell : cells){
        if(!cell->is_ready){
          cell->is_ready = cell->startMultiple(calibrationTime);
        }
        else {
          _cells_ready += 1;
        }
      }
    }

    // Calibrate cells
    for (auto cell : cells){
      cell->setCalFactor(cell->cal_factor);
    }

    tareCells();
  }

  void tareCells(){
    // tare cells
    for (auto cell : cells){
      cell->tareNoDelay();
    }
  }

  void run(){
    // Update data on cells
    for (auto cell : cells){
      cell->update();
    }

    for (auto cell : cells){
      cell->updateForce();
    }

    runned();
  }

  void printCells(){
    for (auto cell : cells){
      printf("%f\t", cell->force);
    }
  }

  void sendCells(){
    printf("!");
    for (auto cell : cells){
      printf("%s=%f;", cell->apelido.c_str(), cell->force);
    }
    printf("@\n");
  }
  private:
    int _cells_ready = 0;
};
