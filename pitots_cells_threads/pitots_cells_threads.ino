#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <HX711_ADC.h>

//ADS1015 constructor
Adafruit_ADS1015 ads(0x49);

//HX711 constructor (DT pin, SCK pin)
HX711_ADC Celula_Horizontal(3, 4);
HX711_ADC Celula_FrontalDireita(5, 6);
HX711_ADC Celula_FrontalEsquerda(7, 8);
HX711_ADC Celula_TraseiraDireita(9, 10);
HX711_ADC Celula_TraseiraEsquerda(11, 12);

const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

class PitotThread: public Thread
{
public:
  float Voltage = 0.0;
  int16_t adc = 0;
  int16_t adc_port;

  void run(){
    adc = ads.readADC_SingleEnded(adc_port);
    Voltage = (adc * 0.1875)/1000;
    runned();
  }
};

class CellsThread: public Thread
{
public:
  float forca_horizontal;
  float forca_frontal_direita;
  float forca_frontal_esquerda;
  float forca_traseira_direita;
  float forca_traseira_esquerda;

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

    // check if tare operations are complete

    if (Celula_Horizontal.getTareStatus() == true) {
      Serial.println("Celula Horizontal tarada");
    }
    if (Celula_FrontalDireita.getTareStatus() == true) {
      Serial.println("Celula Frontal Direita tarada");
    }
    if (Celula_FrontalEsquerda.getTareStatus() == true) {
      Serial.println("Celula Frontal Esquerda tarada");
    }
    if (Celula_TraseiraDireita.getTareStatus() == true) {
      Serial.println("Celula Traseira Direita tarada");
    }
    if (Celula_TraseiraEsquerda.getTareStatus() == true) {
      Serial.println("Celula Traseira Esquerda tarada");
    }
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
};

ThreadController controller = ThreadController();
PitotThread pitot0 = PitotThread();
PitotThread pitot1 = PitotThread();
PitotThread pitot2 = PitotThread();
PitotThread pitot3 = PitotThread();
CellsThread celulas_bancada = CellsThread();

void setup(){
  Serial.begin(115200);

  pitot0.adc_port = 0;
  pitot1.adc_port = 1;
  pitot2.adc_port = 2;
  pitot3.adc_port = 3;

  pitot0.setInterval(1);
  pitot1.setInterval(1);
  pitot2.setInterval(1);
  pitot3.setInterval(1);

  celulas_bancada.initializeCells();
  celulas_bancada.setInterval(1);

  controller.add(&pitot0);
  controller.add(&pitot1);
  controller.add(&pitot2);
  controller.add(&pitot3);
  controller.add(&celulas_bancada);

  ads.begin();
}

void loop(){

  controller.run();

  // printPitots();
  printCells();
  // printPitotAndCells();
  Serial.println();

  receiveCommands();
  interpretCommands();
}

void interpretCommands(){
  if (receivedChars == '!tare@') {
    celulas_bancada.tareCells();
  }
}


void printPitots(){
  printTabbed(1000*pitot0.Voltage);
  printTabbed(1000*pitot1.Voltage);
  printTabbed(1000*pitot2.Voltage);
  printTabbed(1000*pitot3.Voltage);

}

void printCells(){
  printTabbed(celulas_bancada.forca_horizontal);
  printTabbed(celulas_bancada.forca_frontal_direita);
  printTabbed(celulas_bancada.forca_frontal_esquerda);
  printTabbed(celulas_bancada.forca_traseira_direita);
  printTabbed(celulas_bancada.forca_traseira_esquerda);
}

void printTabbed(float value){
  Serial.print(value);
  Serial.print("\t");
}

void printPitotAndCells(){
  printPitots();
  printCells();
}

void sendViaProtocol(){

  Serial.print("!");

  Serial.print("fh");
  Serial.print("=");
  Serial.print(celulas_bancada.forca_horizontal);
  Serial.print(";");

  Serial.print("ffd");
  Serial.print("=");
  Serial.print(celulas_bancada.forca_frontal_direita);
  Serial.print(";");

  Serial.print("ffe");
  Serial.print("=");
  Serial.print(celulas_bancada.forca_frontal_esquerda);
  Serial.print(";");

  Serial.print("ftd");
  Serial.print("=");
  Serial.print(celulas_bancada.forca_traseira_direita);
  Serial.print(";");

  Serial.print("fte");
  Serial.print("=");
  Serial.print(celulas_bancada.forca_traseira_esquerda);
  Serial.print(";");

  Serial.print("pitot0");
  Serial.print("=");
  Serial.print(pitot0.Voltage);
  Serial.print(";");

  Serial.print("pitot1");
  Serial.print("=");
  Serial.print(pitot1.Voltage);
  Serial.print(";");

  Serial.print("pitot2");
  Serial.print("=");
  Serial.print(pitot2.Voltage);
  Serial.print(";");

  Serial.print("pitot3");
  Serial.print("=");
  Serial.print(pitot3.Voltage);
  Serial.print(";");

  Serial.println("@");
}

void receiveCommands() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '!';
  char endMarker = '@';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
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