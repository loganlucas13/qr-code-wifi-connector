/* 
  CODE FOR THE GENERATOR ARDUINO
  This will grab the necessary data (SSID, network type, password) from the
  Network admin Arduino, format it as a string, then as a QR Code, then as a
  bitmap array, then send it off to the Display Arduino for displaying.
  Author: Lance Guevarra, 657508441, lguev2
*/

#include <Wire.h>
#include <SoftwareSerial.h>

// set up third arduino Serial connection
const int rxPin = 2;
const int txPin = 3;

SoftwareSerial displayArduino(rxPin, txPin);

char buf[128]; // create a buffer to read data from, should be enough to hold all of the data necessary
char SSID[16];
char type[8]; // WEP/WPA/nopass
char password[64]; // max out of all types is 63 so we add 1 to allow for the '\0' character
char qrstring[128]; // the string to be made into a qr code

// min time to wait for input. otherwise we check every millisecond and that's not optimal
const int timeInterval = 1000;

// stores the last time we checked
unsigned long lastMillis = 0;

char receive[16]; // what we get from the client
bool stringReceived = false;
bool signalReceived = false;
bool hasBeenSent = false;

void grabDataNeeded(char* buffer){
  Serial.print("buffer: ");
  Serial.println(buffer);
  sscanf(buffer, "%16[^:]:%64[^\n]", SSID, password);
  if (strcmp(password, "none") == 0) {
    strcpy(type, "nopass");
  }
  else {
    strcpy(type, "WPA");
  }

  Serial.print("SSID: ");
  Serial.println(SSID);

  Serial.print("type: ");
  Serial.println(type);

  Serial.print("password: ");
  Serial.println(password);
}


void sendData() {
  displayArduino.write(qrstring);
  hasBeenSent = true;
}

void setup() {
  Serial.begin(9600); // output to terminal
  Serial1.begin(9600); // network manager Arduino
  displayArduino.begin(9600);
}

/*
  How this code works:
  1. we connect to the 1st arduino thru Serial, so that we can grab the network data from it
  2. we convert that data in to a specific format for the 3rd arduino
  3. we connect to the 3rd arduino thru wifi
  4. we send the qr code string to the 3rd arduino and wait for it to give us back some data
  5. once we receive the signal, we send that signal to the 1st one so they can know that someone connected
*/

void loop() {
  unsigned long curMillis = millis();

  if ((curMillis - lastMillis) >= timeInterval) {
    lastMillis = curMillis;
    
    // Step 1: Check for new data from Arduino 1
    if (!stringReceived) {
      if (Serial1.available() > 0) {
        int n = Serial1.readBytesUntil('\n', buf, sizeof(buf) - 1);
        buf[n] = '\0';
        Serial.print("Data received from Arduino 1: ");
        Serial.println(buf);
        stringReceived = true;
      }
    }

    // Step 2: If we have data, process it and prepare QR string
    if (stringReceived && !hasBeenSent) {
      grabDataNeeded(buf);

      if (strcmp(type, "nopass") == 0) {
        sprintf(qrstring, "WIFI:S:%s;T:%s;;", SSID, type);
      } else {
        sprintf(qrstring, "WIFI:S:%s;T:%s;P:%s;;", SSID, type, password);
      }

      Serial.print("QR String: ");
      Serial.println(qrstring);

      sendData();
    }
  }
}

