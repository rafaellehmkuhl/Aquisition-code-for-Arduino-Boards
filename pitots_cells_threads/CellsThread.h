#include <HX711_ADC.h>
#include "Cell.h"
#pragma once

class CellsThread: public Thread
{
public:
  float calibrationTime = 5000; // tare preciscion can be improved by adding a few seconds of stabilising time
  std::vector<Cell> cells;

  CellsThread() :
    Celula_Horizontal(3, 4, 213.0, "Celula_Horizontal"),
    Celula_FrontalDireita(5, 6, 213.0, "Celula_FrontalDireita"),
    Celula_FrontalEsquerda(7, 8, 213.0, "Celula_FrontalEsquerda"),
    Celula_TraseiraDireita(9, 10, 213.0, "Celula_TraseiraDireita"),
    Celula_TraseiraEsquerda(11, 12, 213.0, "Celula_TraseiraEsquerda")
    {
      cells.push_back(Celula_Horizontal);
      cells.push_back(Celula_FrontalDireita);
      cells.push_back(Celula_FrontalEsquerda);
      cells.push_back(Celula_TraseiraDireita);
      cells.push_back(Celula_TraseiraEsquerda);
    }

  void initialize(){

    for (Cell& cell : cells){
      cell.begin();
    }

    while ((Celula_Horizontal.is_ready +
            Celula_FrontalDireita.is_ready +
            Celula_FrontalEsquerda.is_ready +
            Celula_TraseiraDireita.is_ready +
            Celula_TraseiraEsquerda.is_ready) < 5) {

      for (Cell& cell : cells){
        if(!cell.is_ready){
          cell.is_ready = cell.startMultiple(calibrationTime);
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
    Cell Celula_Horizontal;
    Cell Celula_FrontalDireita;
    Cell Celula_FrontalEsquerda;
    Cell Celula_TraseiraDireita;
    Cell Celula_TraseiraEsquerda;
};