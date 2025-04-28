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

// Network Management Arduino
// Description: get network data, send to QR generation Arduino, and display current state + statistics of network from display Arduino.

#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// set up third arduino Serial connection
const int rxPin = 2;
const int txPin = 3;

SoftwareSerial displayArduino(rxPin, txPin);

// size of lcd display
#define NUM_COLS 16
#define NUM_ROWS 2

// constants for pins
const int offLight = 9;
const int onLight = 8;
const int powerButton = 7;
const int generateButton = 6;

// initialize lcd display
LiquidCrystal_I2C lcd(0x27, NUM_COLS, NUM_ROWS);

// millis checking variables
unsigned long prevMillis = 0;
const int interval = 1000;

// debounce variables for both buttons
const int debounceInterval = 50;
unsigned long lastPowerDebounceTime = 0;
unsigned long lastGenerateDebounceTime = 0;
int powerButtonState = LOW;
int generateButtonState = LOW;

// state variables
bool credentialsReceived = false;
bool promptShown = false;
int powerState = LOW; // LOW: off, HIGH: on
int userCount = 0; // number of users connected to network

// data to send to second Arduino
char dataToSend[128];

// basic initialization
void setup() {
  // debugging
  Serial.begin(9600);
  Serial.println("starting setup");
  Serial1.begin(9600);
  displayArduino.begin(9600);

  // get lcd ready for displaying
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // set up pin modes for input/output
  pinMode(offLight, OUTPUT);
  pinMode(onLight, OUTPUT);
  pinMode(powerButton, INPUT);
  pinMode(generateButton, INPUT);

  // turn on off light at beginning
  digitalWrite(offLight, HIGH);

  Serial.println("finished setup");
}

// calls all helper functions
void loop() {
  handlePowerButton();
  handleGenerateButton();

  if (millis() - prevMillis >= interval) {
    prevMillis = millis();

    if (!credentialsReceived) {
      getCredentials();
    }

    updateUserCount();
  }
}

// gathers credentials from Serial input and loads into global buffers
void getCredentials() {
  if (!promptShown) {
    Serial.println("Enter SSID and password in the format of 'SSID:PASSWORD' - if no password, use 'none':");
    promptShown = true;
  }

  // get data from serial monitor
  if (Serial.available() > 0) {
    int availableBytes = Serial.available();
    for (int i = 0; i < availableBytes; i++) {
      dataToSend[i] = Serial.read();
    }
    char buffer[16];
    sscanf(dataToSend, "%16[^:]", buffer);
    lcd.setCursor(0, 0);
    lcd.print(buffer);
    credentialsReceived = true;
  }
}

// perform debounce check, then switch power of system and update led light status
void handlePowerButton() {
  int reading = digitalRead(powerButton);

  if (reading != powerButtonState) {
    lastPowerDebounceTime = millis();
  }

  if (millis() - lastPowerDebounceTime > debounceInterval) {
    if (reading != powerButtonState) {
      powerButtonState = reading;
    }
  }

  if (reading == HIGH && reading != powerButtonState) {
    powerState = !powerState;
    digitalWrite(onLight, powerState);
    digitalWrite(offLight, !powerState);
  }
  powerButtonState = reading;
}

// perform debounce check, then send data to QR code generator
void handleGenerateButton() {
  int reading = digitalRead(generateButton);

  if (reading != generateButtonState) {
    lastGenerateDebounceTime = millis();
  }

  if (millis() - lastGenerateDebounceTime > debounceInterval) {
    if (reading != generateButtonState) {
      generateButtonState = reading;
    }
  }

  if (reading == HIGH && reading != generateButtonState) {
    if (!credentialsReceived) {
      Serial.println("ERROR: no credentials input.");
    }
    else if (!powerState) {
      Serial.println("ERORR: system turned off.");
    }
    else {
      sendData();
    }
  }
  generateButtonState = reading;
}

// send data to QR code generation Arduino
void sendData() {
  Serial.println("sending network information...");
  Serial1.write(dataToSend);
  Serial.println(dataToSend);
  Serial.println("network information sent!\n");
}

// update the user count displayed on the lcd screen
void updateUserCount() {
  if (displayArduino.available() > 0) {
    char c = displayArduino.read();

    // uses 'U' as a marker to update
    if (c == 'U') {
      userCount++;
      char buffer[16];
      sprintf(buffer, "# Users: %d", userCount);
      lcd.setCursor(0, 1);
      lcd.print(buffer);
    }
  }
}
