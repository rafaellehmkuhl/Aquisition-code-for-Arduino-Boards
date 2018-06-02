#include "Arduino.h"
#include <ArduinoSTL.h>

class BancadaFunctions
{
public:
    static const byte numChars = 32;
    char receivedChars[numChars];
    bool newData = false;

    void BancadaFunctions::interpretCommands(CellsThread &celulas_bancada, bool &print_pitots, bool &print_cells, bool &send_outside){
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

    void BancadaFunctions::printPitots(int numPitotBoards, std::vector<PitotBoardThread> &pitots){
        for (int i=0; i<4*numPitotBoards; i++){
            printTabbed(1000*pitots[i].Voltage);
        }
    }

    void BancadaFunctions::printCells(CellsThread &celulas_bancada){
        printTabbed(celulas_bancada.forca_horizontal);
        printTabbed(celulas_bancada.forca_frontal_direita);
        printTabbed(celulas_bancada.forca_frontal_esquerda);
        printTabbed(celulas_bancada.forca_traseira_direita);
        printTabbed(celulas_bancada.forca_traseira_esquerda);
    }

    void BancadaFunctions::printTabbed(float value){
        printf("%f\t", value);
    }

    void BancadaFunctions::sendDataViaProtocol(bool send_cells_via_protocol, bool send_pitots_via_protocol, int numPitotBoards, std::vector<PitotBoardThread> &pitots, CellsThread &celulas_bancada){

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

    void BancadaFunctions::printProtocolled(String apelido, float value){
        printf("%s=%f;", apelido.c_str(), value);
    }

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