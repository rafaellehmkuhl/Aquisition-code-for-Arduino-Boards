#pragma once
#include "Arduino.h"
#include <ArduinoSTL.h>

class BancadaFunctions
{
public:
    static const byte numChars = 32;
    char receivedChars[numChars];
    bool newData = false;

    // void BancadaFunctions::interpretCommands(CellsThread &celulas_bancada, bool &print_pitots, bool &print_cells, bool &send_outside){
    //     if (receivedChars == '!tare_cells@') {
    //         celulas_bancada.tareCells();
    //     }
    //     if (receivedChars == '!print_pitots@') {
    //         print_pitots = !print_pitots;
    //     }
    //     if (receivedChars == '!print_cells@') {
    //         print_cells = !print_cells;
    //     }
    //     if (receivedChars == '!send_outside@') {
    //         send_outside = !send_outside;
    //     }
    // }

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