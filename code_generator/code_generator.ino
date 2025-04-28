// Logan Lucas, Henry Chen, Lance Guevarra
// netids: lluca5, hchen250, lguev2
// UINs: 667695865, 675227500, 657508441

// Group Number: 23

// Project Title: WiFi QR Code Connector

// Abstract: 
// Arduino system that enables WiFi access through automatically generating QR codes
// using three Arduinos to manage network setup, QR code generation, and display.
// It is an independent system for homeowners aiming to simplify the process to access home
// internet for their guests

// QR Code Generator Arduino
// Description: Grabs data from network management Arduino, parse it, format it, and then send to display Arduino.

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

bool stringReceived = false;
bool signalReceived = false;
bool hasBeenSent = false;

void grabDataNeeded(char* buffer){
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

// sends parsed data to the display Arduino
void sendData() {
  displayArduino.write(qrstring);
  hasBeenSent = true; // make sure we only send once
}

void setup() {
  Serial.begin(9600); // output to terminal
  Serial1.begin(9600); // network manager Arduino
  displayArduino.begin(9600);
}


void loop() {
  unsigned long curMillis = millis();

  if ((curMillis - lastMillis) >= timeInterval) {
    lastMillis = curMillis;
    
    // check for data from Arduino 1
    if (!stringReceived) {
      if (Serial1.available() > 0) {
        int n = Serial1.readBytesUntil('\n', buf, sizeof(buf) - 1);
        buf[n] = '\0';
        Serial.print("Data received from Arduino 1: ");
        Serial.println(buf);
        stringReceived = true;
      }
    }

    // if we have unsent data, parse it and send to display Arduino
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

