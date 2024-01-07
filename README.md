# Arduino UNO SIM800C GPS Tracker
 * Created By  : Enes İYİDİL
 * Create On   : 12.09.2023
 *
 * Last Update : 16.12.2023
# UNO Pin Out 
 * Circuit Schematic Comments:
 *
 * Vin 2S Li-Ion battery
 * Connection: Can be connected to Arduino's Vin pin.
 *
 * D0 GPS(RX) and D1 GPS(TX)
 * Connection: RX pin of the GPS module is connected to Arduino's D0 pin, while TX pin is connected to D1 pin.
 *
 * D4 Reset Button input; the other leg of the button to 5V
 * Connection: One leg of the button is connected to D4, the other leg to 5V.
 *
 * D5 Buzzer output; the other leg to GND
 * Connection: One leg of the buzzer is connected to D5, the other leg to GND.
 *
 * D6 GSM Status Led; the other leg via 220ohm resistor to GND
 * Connection: Anode of the LED to D6, cathode via a 220 ohm resistor to GND.
 *
 * D7 GSM button
 * Connection: One leg of the button to D7, the other leg to GND.
 *
 * D8 SW-420 input
 * Connection: Signal pin of SW-420 vibration sensor to D8.
 *
 * D10 GSM(RX) and D11 GSM(TX)
 * Connection: RX pin of GSM module to Arduino's D10 pin, TX pin to D11 pin.
 *
 * D13 Security LED; the other leg via 220ohm resistor to GND
 * Connection: Anode of the LED to D13, cathode via a 220 ohm resistor to GND.
