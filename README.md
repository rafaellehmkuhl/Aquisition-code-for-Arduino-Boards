Bancada Mitica 2.0 - Aquisition Code for Arduino Boards

The Bancada Mitica 2.0 eletronics use the concept of [Sensor Hubs](https://en.wikipedia.org/wiki/Sensor_hub), this is, there's one or more boards responsible for adquiring data from sensors, called Sources, and there's one central board responsible for receiving data from Sources, processing and forwarding it. This central board is called Sink.

This repository contais the code for the Source boards.

This Source boards currently include Load Cells (through HX711) and Pitots (through MPXV7002DP and ADS1115).
