#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <HX711_ADC.h>
#include <MS5611.h>
#include "I2Cdev.h"
#include "MPU6050.h"

const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;
boolean can_send = true;

boolean use_pitots = true;
boolean print_pitots = false;
int numPitotBoards = 1;

boolean use_cells = true;
boolean print_cells = false;

boolean use_baro = false;
boolean print_baro = false;

boolean use_accgyro = true;
boolean print_accgyro = false;

boolean send_outside = true;

float checksum = 0;

float previousTime = 0;
boolean print_time = false;

//MPU6050 constructor
MPU6050 accelgyro;

//MS5611 constructor
MS5611 baro;

//ADS1015 constructor
Adafruit_ADS1115 ads0(0x48);
Adafruit_ADS1115 ads1(0x49);
Adafruit_ADS1115 ads2(0x4A);
Adafruit_ADS1115 ads3(0x4B);

//HX711 constructor (DT pin, SCK pin)
HX711_ADC Celula_Horizontal(3, 4);
HX711_ADC Celula_FrontalDireita(5, 6);
HX711_ADC Celula_FrontalEsquerda(7, 8);
HX711_ADC Celula_TraseiraDireita(9, 10);
HX711_ADC Celula_TraseiraEsquerda(11, 12);

class AccGyroThread: public Thread
{
public:

  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  float ax_f, ay_f, az_f;
  float gx_f, gy_f, gz_f;

  void initializeAccGyro(){
    accelgyro.initialize();
  }

  void run(){
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    ax_f = ax/1670.0; 
    ay_f = ay/1670.0; 
    az_f = az/1670.0;
    gx_f = gx/262.0;
    gy_f = gy/262.0;
    gz_f = gz/262.0;
    
    runned();
  }
};

class BaroThread: public Thread
{
public:

  float pressure;
  float temperature;

  void initializeBaro(){
    baro = MS5611();
    baro.begin();
  }

  void run(){
    pressure = baro.getPressure() / 1000.0;
    temperature = baro.getTemperature() / 100.0;
    
    runned();
  }
};

class PitotThread: public Thread
{
public:
  float Voltage = 0.0;
  int16_t adc = 0;
  int16_t adc_port;
  int16_t board_num;

  void run(){
    if (board_num == 0){
      adc = ads0.readADC_SingleEnded(adc_port);
    }
    else if (board_num == 1){
      adc = ads1.readADC_SingleEnded(adc_port);
    }
    else if (board_num == 2){
      adc = ads2.readADC_SingleEnded(adc_port);
    }
    else if (board_num == 3){
      adc = ads3.readADC_SingleEnded(adc_port);
    }

    Voltage = (adc * 0.1875);
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
    Celula_Horizontal.setCalFactor(1.0); // user set calibration factor (float)
    Celula_FrontalDireita.setCalFactor(1.0); // user set calibration factor (float)
    Celula_FrontalEsquerda.setCalFactor(1.0); // user set calibration factor (float)
    Celula_TraseiraDireita.setCalFactor(1.0); // user set calibration factor (float)
    Celula_TraseiraEsquerda.setCalFactor(1.0); // user set calibration factor (float)

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
PitotThread pitot0 = PitotThread();
PitotThread pitot1 = PitotThread();
PitotThread pitot2 = PitotThread();
PitotThread pitot3 = PitotThread();
PitotThread pitot4 = PitotThread();
PitotThread pitot5 = PitotThread();
PitotThread pitot6 = PitotThread();
PitotThread pitot7 = PitotThread();
PitotThread pitot8 = PitotThread();
PitotThread pitot9 = PitotThread();
PitotThread pitot10 = PitotThread();
PitotThread pitot11 = PitotThread();
PitotThread pitot12 = PitotThread();
PitotThread pitot13 = PitotThread();
PitotThread pitot14 = PitotThread();
PitotThread pitot15 = PitotThread();
CellsThread celulas_bancada = CellsThread();
BaroThread barometer = BaroThread();
AccGyroThread accgyro = AccGyroThread();

void setup(){
  Serial.begin(115200);

  if (use_pitots == true){

    if (numPitotBoards >= 1){
      //ADS1015 constructor
      pitot0.board_num = 0;
      pitot1.board_num = 0;
      pitot2.board_num = 0;
      pitot3.board_num = 0;

      pitot0.adc_port = 0;
      pitot1.adc_port = 1;
      pitot2.adc_port = 2;
      pitot3.adc_port = 3;

      pitot0.setInterval(1);
      pitot1.setInterval(1);
      pitot2.setInterval(1);
      pitot3.setInterval(1);

      ads0.begin();

      controller.add(&pitot0);
      controller.add(&pitot1);
      controller.add(&pitot2);
      controller.add(&pitot3);
    }
    if (numPitotBoards >= 2){
      //ADS1015 constructor
      pitot4.board_num = 1;
      pitot5.board_num = 1;
      pitot6.board_num = 1;
      pitot7.board_num = 1;

      pitot4.adc_port = 0;
      pitot5.adc_port = 1;
      pitot6.adc_port = 2;
      pitot7.adc_port = 3;

      pitot4.setInterval(1);
      pitot5.setInterval(1);
      pitot6.setInterval(1);
      pitot7.setInterval(1);

      ads1.begin();

      controller.add(&pitot4);
      controller.add(&pitot5);
      controller.add(&pitot6);
      controller.add(&pitot7);
    }
    if (numPitotBoards >= 3){
      //ADS1015 constructor
      pitot8.board_num = 2;
      pitot9.board_num = 2;
      pitot10.board_num = 2;
      pitot11.board_num = 2;

      pitot8.adc_port = 0;
      pitot9.adc_port = 1;
      pitot10.adc_port = 2;
      pitot11.adc_port = 3;

      pitot8.setInterval(1);
      pitot9.setInterval(1);
      pitot10.setInterval(1);
      pitot11.setInterval(1);

      ads2.begin();

      controller.add(&pitot8);
      controller.add(&pitot9);
      controller.add(&pitot10);
      controller.add(&pitot11);
    }
    if (numPitotBoards >= 4){
      //ADS1015 constructor
      pitot12.board_num = 3;
      pitot13.board_num = 3;
      pitot14.board_num = 3;
      pitot15.board_num = 3;

      pitot12.adc_port = 0;
      pitot13.adc_port = 1;
      pitot14.adc_port = 2;
      pitot15.adc_port = 3;

      pitot12.setInterval(1);
      pitot13.setInterval(1);
      pitot14.setInterval(1);
      pitot15.setInterval(1);

      ads3.begin();

      controller.add(&pitot12);
      controller.add(&pitot13);
      controller.add(&pitot14);
      controller.add(&pitot15);
    }
  }

  if(use_cells == true){
    celulas_bancada.initializeCells();
    celulas_bancada.setInterval(1);
    controller.add(&celulas_bancada);
  }

  if(use_baro == true){
    barometer.initializeBaro();
    barometer.setInterval(1);
    controller.add(&barometer);
  }

  if(use_accgyro == true){
    accgyro.initializeAccGyro();
    accgyro.setInterval(1);
    controller.add(&accgyro);
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

  if (print_baro){
    printBaro();
  }

  if (print_accgyro){
    printAccGyro();
  }

  if (print_pitots ||  print_cells || print_baro || print_accgyro){
    Serial.println();
  }

  if (send_outside){
    sendDataViaProtocol();
  }

  receiveCommands();
  interpretCommands();

  if (print_time) {
    Serial.println(1000.0 / (millis() - previousTime));
    previousTime = millis(); 
  }
}

void interpretCommands(){
  if (newData){
    if (strcmp(receivedChars, "tc") == 0) {
      celulas_bancada.tareCells();
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
    if (strcmp(receivedChars, "cs") == 0) {
      can_send = true;
    }
    newData = false;
  }
}


void printPitots(){
  if (numPitotBoards >= 1){
    printTabbed(pitot0.Voltage);
    printTabbed(pitot1.Voltage);
    printTabbed(pitot2.Voltage);
    printTabbed(pitot3.Voltage);
  }
  if (numPitotBoards >= 2){
    printTabbed(pitot4.Voltage);
    printTabbed(pitot5.Voltage);
    printTabbed(pitot6.Voltage);
    printTabbed(pitot7.Voltage);
  }
  if (numPitotBoards >= 3){
    printTabbed(pitot8.Voltage);
    printTabbed(pitot9.Voltage);
    printTabbed(pitot10.Voltage);
    printTabbed(pitot11.Voltage);
  }
  if (numPitotBoards >= 4){
    printTabbed(pitot12.Voltage);
    printTabbed(pitot13.Voltage);
    printTabbed(pitot14.Voltage);
    printTabbed(pitot15.Voltage);
  }
}

void printCells(){
  printTabbed(celulas_bancada.forca_horizontal);
  printTabbed(celulas_bancada.forca_frontal_direita);
  printTabbed(celulas_bancada.forca_frontal_esquerda);
  printTabbed(celulas_bancada.forca_traseira_direita);
  printTabbed(celulas_bancada.forca_traseira_esquerda);
}

void printBaro(){
  printTabbed(barometer.pressure);
  printTabbed(barometer.temperature);
}

void printAccGyro(){
  printTabbed(accgyro.ax_f);
  printTabbed(accgyro.ay_f);
  printTabbed(accgyro.az_f);
  printTabbed(accgyro.gx_f);
  printTabbed(accgyro.gy_f);
  printTabbed(accgyro.gz_f);
}

void printTabbed(float value){
  Serial.print(value);
  Serial.print("\t");
}

void sendDataViaProtocol(){

  if (can_send) {
    Serial.print("!");

    printProtocolled("fh", celulas_bancada.forca_horizontal);
    printProtocolled("ffd", celulas_bancada.forca_frontal_direita);
    printProtocolled("ffe", celulas_bancada.forca_frontal_esquerda);
    printProtocolled("ftd", celulas_bancada.forca_traseira_direita);
    printProtocolled("fte", celulas_bancada.forca_traseira_esquerda);

    if (numPitotBoards >= 1){
      printProtocolled("pt0", pitot0.Voltage);
      printProtocolled("pt1", pitot1.Voltage);
      printProtocolled("pt2", pitot2.Voltage);
      printProtocolled("pt3", pitot3.Voltage);
    }
    if (numPitotBoards >= 2){
      printProtocolled("pt4", pitot4.Voltage);
      printProtocolled("pt5", pitot5.Voltage);
      printProtocolled("pt6", pitot6.Voltage);
      printProtocolled("pt7", pitot7.Voltage);
    }
    if (numPitotBoards >= 3){
      printProtocolled("pt8", pitot8.Voltage);
      printProtocolled("pt9", pitot9.Voltage);
      printProtocolled("pt10", pitot10.Voltage);
      printProtocolled("pt11", pitot11.Voltage);
    }
    if (numPitotBoards >= 4){
      printProtocolled("pt12", pitot12.Voltage);
      printProtocolled("pt13", pitot13.Voltage);
      printProtocolled("pt14", pitot14.Voltage);
      printProtocolled("pt15", pitot15.Voltage);
    }

    printProtocolled("prs", barometer.pressure);
    printProtocolled("tmp", barometer.temperature);

    printProtocolled("acx", accgyro.ax_f);
    printProtocolled("acy", accgyro.ay_f);
    printProtocolled("acz", accgyro.az_f);
    printProtocolled("gyx", accgyro.gx_f);
    printProtocolled("gyy", accgyro.gy_f);
    printProtocolled("gyz", accgyro.gz_f);

    printChecksum("cks", checksum);

    Serial.println("@");

    can_send = false;
  }
}

void printChecksum(String apelido, float value){
  Serial.print(apelido);
  Serial.print("=");
  Serial.print(value);
  checksum = 0;
}

void printProtocolled(String apelido, float value){
  Serial.print(apelido);
  Serial.print("=");
  Serial.print(value);
  Serial.print(";");

  checksum += value;
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
