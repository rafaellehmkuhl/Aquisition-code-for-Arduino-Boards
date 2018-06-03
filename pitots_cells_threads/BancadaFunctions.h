#include "Arduino.h"
#include <ArduinoSTL.h>

class BancadaFunctions
{
public:
    static const byte numChars = 32;
    char receivedChars[numChars];
    bool newData = false;


    void BancadaFunctions::receiveCommands() {
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
};