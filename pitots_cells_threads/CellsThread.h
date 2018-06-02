#include <HX711_ADC.h>

class CellsThread: public Thread
{
public:
  float forca_horizontal;
  float forca_frontal_direita;
  float forca_frontal_esquerda;
  float forca_traseira_direita;
  float forca_traseira_esquerda;

  CellsThread() :
    Celula_Horizontal(3, 4),
    Celula_FrontalDireita(5, 6),
    Celula_FrontalEsquerda(7, 8),
    Celula_TraseiraDireita(9, 10),
    Celula_TraseiraEsquerda(11, 12)
  {}

  void initializeCells(){
    Celula_Horizontal.begin();
    Celula_FrontalDireita.begin();
    Celula_FrontalEsquerda.begin();
    Celula_TraseiraDireita.begin();
    Celula_TraseiraEsquerda.begin();

    long calibrationTime = 5000; // tare preciscion can be improved by adding a few seconds of stabilising time

    byte Celula_Horizontal_ready = 0;
    byte Celula_FrontalDireita_ready = 0;
    byte Celula_FrontalEsquerda_ready = 0;
    byte Celula_TraseiraDireita_ready = 0;
    byte Celula_TraseiraEsquerda_ready = 0;

    // Run startup, stabilization and tare, all modules simultaniously
    while ((Celula_Horizontal_ready + Celula_FrontalDireita_ready + Celula_FrontalEsquerda_ready + Celula_TraseiraDireita_ready + Celula_TraseiraEsquerda_ready) < 5) {
      if (!Celula_Horizontal_ready) Celula_Horizontal_ready = Celula_Horizontal.startMultiple(calibrationTime);
      if (!Celula_FrontalDireita_ready) Celula_FrontalDireita_ready = Celula_FrontalDireita.startMultiple(calibrationTime);
      if (!Celula_FrontalEsquerda_ready) Celula_FrontalEsquerda_ready = Celula_FrontalEsquerda.startMultiple(calibrationTime);
      if (!Celula_TraseiraDireita_ready) Celula_TraseiraDireita_ready = Celula_TraseiraDireita.startMultiple(calibrationTime);
      if (!Celula_TraseiraEsquerda_ready) Celula_TraseiraEsquerda_ready = Celula_TraseiraEsquerda.startMultiple(calibrationTime);
    }

    // Calibrate cells
    Celula_Horizontal.setCalFactor(2581.0); // user set calibration factor (float)
    Celula_FrontalDireita.setCalFactor(744.0); // user set calibration factor (float)
    Celula_FrontalEsquerda.setCalFactor(744.0); // user set calibration factor (float)
    Celula_TraseiraDireita.setCalFactor(744.0); // user set calibration factor (float)
    Celula_TraseiraEsquerda.setCalFactor(744.0); // user set calibration factor (float)

    Serial.println("Startup and tare are complete");
  }

  void tareCells(){

    // tare cells
    Celula_Horizontal.tareNoDelay();
    Celula_FrontalDireita.tareNoDelay();
    Celula_FrontalEsquerda.tareNoDelay();
    Celula_TraseiraDireita.tareNoDelay();
    Celula_TraseiraEsquerda.tareNoDelay();
  }

  void run(){
    // Update data on cells
    Celula_Horizontal.update();
    Celula_FrontalDireita.update();
    Celula_FrontalEsquerda.update();
    Celula_TraseiraDireita.update();
    Celula_TraseiraEsquerda.update();

    // Get data from cells
    forca_horizontal = Celula_Horizontal.getData();
    forca_frontal_direita = Celula_FrontalDireita.getData();
    forca_frontal_esquerda = Celula_FrontalEsquerda.getData();
    forca_traseira_direita = Celula_TraseiraDireita.getData();
    forca_traseira_esquerda = Celula_TraseiraEsquerda.getData();

    runned();
  }

  private:
    HX711_ADC Celula_Horizontal;
    HX711_ADC Celula_FrontalDireita;
    HX711_ADC Celula_FrontalEsquerda;
    HX711_ADC Celula_TraseiraDireita;
    HX711_ADC Celula_TraseiraEsquerda;
};