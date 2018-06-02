class PitotBoardThread: public Thread
{
public:
  float Voltage = 0.0;
  int16_t adc = 0;
  int adc_port;
  String apelido;
  Adafruit_ADS1015 &ads;
  PitotBoardThread(){};
  PitotBoardThread(int adc_port, String apelido, Adafruit_ADS1015 &ads){};

  void run(){
    adc = ads.readADC_SingleEnded(adc_port);
    Voltage = (adc * 0.1875)/1000;
    runned();
  }
};