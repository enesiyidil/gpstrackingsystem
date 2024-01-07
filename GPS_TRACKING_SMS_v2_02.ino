/* ****************************************************************************************************************************
 *
 * Created By  : Enes İYİDİL
 * Create On   : 12.09.2023
 *
 * Last Update : 16.12.2023
 *
 * Arduino UNO SIM800C GPS Tracker
 *
 * | -------------------------------------------------------------- UNO Pin Out -------------------------------------------------------------- |
 * | ----------------------------------------------------------------------------------------------------------------------------------------- |
 * | D0      | D1      | D2      | D3      | D4      | D5      | D6      | D7      | D8      | D9      | D10     | D11     | D12     | D13     | 
 * | GPS(RX) | GPS(TX) |         |         | Reset B | Buzzer  | GSM LED | GSM B   | SW-420  |         | GSM(RX) | GSM(TX) |         | S LED   |
 * | ----------------------------------------------------------------------------------------------------------------------------------------- |
 * | A0      | A1      | A2      | A3      | A4      | A5      | ----------------------------------------------------------------------------- | 
 * |         |         |         |         |         |         | ----------------------------------------------------------------------------- | 
 * | ----------------------------------------------------------------------------------------------------------------------------------------- |
 *
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
 *
 * ****************************************************************************************************************************/

#include <RobonioGSM.h>
#include <EEPROM.h>
#include <TinyGPSPlus.h>
#include <math.h>

// ****************************************************************************************************************************

#define RESET_BUTTON 4
#define BUZZER 5
#define GSM_STATUS_LED 6
#define GSM_SWITCH 7
#define SW_420 8
#define LED_SECURITY 13

// ****************************************************************************************************************************

#define MESSAGE_SYSTEM_OPENED "System opened."
#define MESSAGE_SECURITY_ACTIVE "Security active."
#define MESSAGE_SECURITY_CLOSED "Security closed."
#define MESSAGE_SAVED "Saved number."

// ****************************************************************************************************************************

#define CHECK_OPEN "OPEN"
#define CHECK_CLOSE "CLOSE"
#define CHECK_WHERE_1 "WHERE"
#define CHECK_WHERE_2 "WHERE?"
#define CHECK_WHERE_3 "LOCATION"
#define CHECK_SAVE "SAVE"
#define CHECK_SING "SING"

// ****************************************************************************************************************************

static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
RobonioGSM Robonio;
 
// ****************************************************************************************************************************

String smsText;
String smsNumber;
char phoneNumber[14];
unsigned int phoneNumberAddr = 0;

// ****************************************************************************************************************************

const double EarthRadiusKm = 6371.0;
String mapText = "https://www.google.com/maps?q=";
unsigned long startTime = 0;
unsigned long sendTime = 60 * 1000;

// ****************************************************************************************************************************

int singMelody[] = {262, 294, 330, 349, 392, 440, 494, 523};
int singNoteDurations[] = {250, 250, 250, 250, 250, 250, 250, 250};

int securityMelody[] = {
  262, 294, 330, 392, 392, 440, 392, 0,
  330, 349, 392, 330, 262, 294, 262, 0,
  330, 392, 523, 494, 440, 392, 0
};
int securityNoteDurations[] = {
  250, 250, 250, 500, 250, 250, 500, 500,
  250, 250, 250, 500, 250, 250, 500, 500,
  250, 250, 250, 250, 250, 500, 500
};

int openingMelody[] = {
  500, 0, 500, 0, 500, 0, 500, 0, 500, 0, 500, 0, 500, 0, 500, 0,
  600, 0, 600, 0, 600, 0, 600, 0,
  700, 0, 700, 0, 700, 0, 700, 0,
  800, 0, 800, 0, 800, 0, 800, 0,
  900, 0, 900, 0, 900, 0, 900, 0
};
int openingNoteDurations[] = {
  100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
  60, 60, 60, 60, 60, 60, 60, 60,
  40, 40, 40, 40, 40, 40, 40, 40,
  20, 20, 20, 20, 20, 20, 20, 20,
  10, 10, 10, 10, 10, 10, 10, 10
};

// ****************************************************************************************************************************

float latS, lngS;
float lat, lng;
volatile bool isSecurityActive = true;

// ****************************************************************************************************************************

void setup() {
  // Required pin settings (I/O)
  pinMode(RESET_BUTTON, INPUT);
  pinMode(SW_420, INPUT);
  
  pinMode(BUZZER, OUTPUT);
  pinMode(GSM_STATUS_LED, OUTPUT);
  pinMode(GSM_SWITCH, OUTPUT);
  pinMode(LED_SECURITY, OUTPUT);

  // Activate GSM
  digitalWrite(GSM_SWITCH, HIGH);
  delay(3000);
  digitalWrite(GSM_SWITCH, LOW);

  // Starting GPS communication
  Serial.begin(GPSBaud);
  smartDelay(5000);

  EEPROM.put(phoneNumberAddr, "+905059741244");
  // Get phone number from EEPROM
  EEPROM.get(phoneNumberAddr, phoneNumber);
  //phoneNumber = readEEPROM(phoneNumberAddr, 13);
  
  // Starting GSM communication
  Robonio.start();
  delay(2000);
  Robonio.allSmsDelete();
  smartDelay(2000);

  // Send information
  Robonio.smsSend(phoneNumber, MESSAGE_SYSTEM_OPENED);
  smartDelay(1000);
  Robonio.smsSend(phoneNumber, MESSAGE_SECURITY_ACTIVE);
  digitalWrite(LED_SECURITY, HIGH);
  playMelody(openingMelody, openingNoteDurations);
}

void loop() {
  smsText = Robonio.smsRead(1);
  Robonio.callAnswer();

  if(smsText.length() > 0){
    smsNumber = Robonio.NumberSmsRead(1);
    smsText.toUpperCase();

    if (smsText.indexOf(CHECK_OPEN) != -1) {
      digitalWrite(LED_SECURITY, HIGH);
      Robonio.smsSend(phoneNumber, MESSAGE_SECURITY_ACTIVE);
      playMelody(securityMelody, securityNoteDurations);

    }else if (smsText.indexOf(CHECK_CLOSE) != -1) {
      digitalWrite(LED_SECURITY, LOW);
      Robonio.smsSend(phoneNumber, MESSAGE_SECURITY_CLOSED);
      digitalWrite(BUZZER, HIGH);
      smartDelay(1000);
      digitalWrite(BUZZER, LOW);

    }else if (smsText.indexOf(CHECK_WHERE_1) != -1 || smsText.indexOf(CHECK_WHERE_2) != -1 || smsText.indexOf(CHECK_WHERE_3) != -1) {
      sendData();

    }else if(smsText.indexOf(CHECK_SAVE) != -1){
      EEPROM.put(phoneNumberAddr, smsNumber);
      delay(10);
      EEPROM.get(phoneNumberAddr, phoneNumber);
      Robonio.smsSend(phoneNumber, MESSAGE_SAVED);

    }else if(smsText.indexOf(CHECK_SING) != -1){
       playMelody(singMelody, singNoteDurations);

    }
    else {
    }
    Robonio.allSmsDelete();
  }

  if(isSecurityActive){
    lat = gps.location.lat();
    lng = gps.location.lng();
    double distance = haversine(latS, lngS, lat, lng);
    if(distance > 15){
      if(millis() - startTime > sendTime){
        sendData();
        startTime = millis();
      }
    }
  }

  smartDelay(10);
}

static void smartDelay(unsigned long ms) {
	unsigned long start = millis();
	do {
		while (Serial.available()) {
			gps.encode(Serial.read());
		}
	} while (millis() - start < ms);
}

double haversine(double lat1, double lon1, double lat2, double lon2) {
  double lat1Rad = radians(lat1);
  double lon1Rad = radians(lon1);
  double lat2Rad = radians(lat2);
  double lon2Rad = radians(lon2);
  double latDiff = lat2Rad - lat1Rad;
  double lonDiff = lon2Rad - lon1Rad;
  double a = pow(sin(latDiff / 2), 2) + cos(lat1Rad) * cos(lat2Rad) * pow(sin(lonDiff / 2), 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  double distance = (EarthRadiusKm * c) * 1000;
  return distance;
}

void sendData(){
  String data = mapText + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
  unsigned int dataLen = data.length() + 1;
  char dataArr[dataLen];
  data.toCharArray(dataArr, dataLen);
  Robonio.smsSend(phoneNumber, dataArr);
}

void playMelody(int melody[], int noteDurations[]) {
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
    if (melody[i] == 0) {
      noTone(BUZZER); // Sesi kapat
    } else {
      tone(BUZZER, melody[i], noteDurations[i]); // Belirli frekansta ses çal
      delay(noteDurations[i] + 30); // Notalar arasında bekleme süresi
    }
  }
}
