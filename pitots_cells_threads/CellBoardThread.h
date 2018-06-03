#pragma once
#include <HX711_ADC.h>
#include "Cell.h"

class CellBoardThread: public Thread
{
public:
  float calibrationTime = 5000; // tare preciscion can be improved by adding a few seconds of stabilising time
  std::vector<Cell> cells;

  CellBoardThread() :
    cells{
      Cell(3, 4, 213.0, "Celula_Horizontal"),
      Cell(5, 6, 213.0, "Celula_FrontalDireita"),
      Cell(7, 8, 213.0, "Celula_FrontalEsquerda"),
      Cell(9, 10, 213.0, "Celula_TraseiraDireita"),
      Cell(11, 12, 213.0, "Celula_TraseiraEsquerda")
    }
    {
      initialize();
    }

  void initialize(){

    for (Cell& cell : cells){
      cell.begin();
    }

    while (_cells_ready < 5){
      _cells_ready = 0;
      for (Cell& cell : cells){
        if(!cell.is_ready){
          cell.is_ready = cell.startMultiple(calibrationTime);
        }
        else {
          _cells_ready += 1;
        }
      }
    }

    // Calibrate cells
    for (Cell& cell : cells){
      cell.setCalFactor(cell.cal_factor);
    }

    tareCells();
  }

  void tareCells(){
    // tare cells
    for (Cell& cell : cells){
      cell.tareNoDelay();
    }
  }

  void run(){
    // Update data on cells
    for (Cell& cell : cells){
      cell.update();
    }

    for (Cell& cell : cells){
      cell.updateForce();
    }

    runned();
  }

  void printCells(){
    for (Cell& cell : cells){
      printf("%f\t", cell.force);
    }
  }

  void sendCells(){
    printf("!");
    for (Cell& cell : cells){
      printf("%s=%f;", cell.apelido.c_str(), cell.force);
    }
    printf("@\n");
  }
  private:
    bool _cells_ready = 0;
};