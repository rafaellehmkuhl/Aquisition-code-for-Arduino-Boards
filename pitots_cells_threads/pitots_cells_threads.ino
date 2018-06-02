#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <HX711_ADC.h>

#include "PitotThread.h"
#include "CellsThread.h"

const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

boolean use_pitots = false;
boolean print_pitots = false;
boolean send_pitots_via_protocol = false;
int numPitotBoards = 1;

boolean use_cells = true;
boolean print_cells = true;
boolean send_cells_via_protocol = false;

boolean send_outside = false;

//ADS1015 constructor
Adafruit_ADS1015 ads0(0x48);
Adafruit_ADS1015 ads1(0x49);
Adafruit_ADS1015 ads2(0x4A);
Adafruit_ADS1015 ads3(0x4B);

//HX711 constructor (DT pin, SCK pin)
HX711_ADC Celula_Horizontal(3, 4);
HX711_ADC Celula_FrontalDireita(5, 6);
HX711_ADC Celula_FrontalEsquerda(7, 8);
HX711_ADC Celula_TraseiraDireita(9, 10);
HX711_ADC Celula_TraseiraEsquerda(11, 12);

ThreadController controller = ThreadController();

PitotThread pitots[] = {
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

CellsThread celulas_bancada = CellsThread(Celula_Horizontal,
                                          Celula_FrontalDireita,
                                          Celula_FrontalEsquerda,
                                          Celula_TraseiraDireita,
                                          Celula_TraseiraEsquerda);

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
    printPitots();
  }

  if (print_cells){
    printCells();
  }

  printf("\n");

  if (send_outside){
    sendDataViaProtocol();
  }

  receiveCommands();
  interpretCommands();
}

void interpretCommands(){
  if (receivedChars == '!tare_cells@') {
    celulas_bancada.tareCells();
  }
  if (receivedChars == '!print_pitots@') {
    print_pitots = !print_pitots;
  }
  if (receivedChars == '!print_cells@') {
    print_cells = !print_cells;
  }
  if (receivedChars == '!send_outside@') {
    send_outside = !send_outside;
  }
}


void printPitots(){
  for (int i=0; i<4*numPitotBoards; i++){
    printTabbed(1000*pitots[i].Voltage);
  }
}

void printCells(){
  printTabbed(celulas_bancada.forca_horizontal);
  printTabbed(celulas_bancada.forca_frontal_direita);
  printTabbed(celulas_bancada.forca_frontal_esquerda);
  printTabbed(celulas_bancada.forca_traseira_direita);
  printTabbed(celulas_bancada.forca_traseira_esquerda);
}

void printTabbed(float value){
  printf("%f\t", value);
}

void sendDataViaProtocol(){

  printf("!");

  if(send_cells_via_protocol){
    printProtocolled("fh", celulas_bancada.forca_horizontal);
    printProtocolled("ffd", celulas_bancada.forca_frontal_direita);
    printProtocolled("ffe", celulas_bancada.forca_frontal_esquerda);
    printProtocolled("ftd", celulas_bancada.forca_traseira_direita);
    printProtocolled("fte", celulas_bancada.forca_traseira_esquerda);
  }

  if(send_pitots_via_protocol){
    for (int i=0; i<4*numPitotBoards; i++){
      printProtocolled(pitots[i].apelido, 1000*pitots[i].Voltage);
    }
  }

  printf("@\n");
}

void printProtocolled(String apelido, float value){
  printf("%s=%f;", apelido.c_str(), value);
}

void receiveCommands() {
  static boolean recvInProgress = false;
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