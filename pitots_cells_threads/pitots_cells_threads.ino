#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <HX711_ADC.h>

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

class PitotThread: public Thread
{
public:
  float Voltage = 0.0;
  int16_t adc = 0;
  int adc_port;
  String apelido;
  Adafruit_ADS1015 &ads;
  PitotThread(){};
  PitotThread(int adc_port, String apelido, Adafruit_ADS1015 &ads){};

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
  printf("%s=%f;", apelido, value);
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