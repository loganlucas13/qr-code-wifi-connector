// Network Management Arduino
// Purpose: get network data, send to QR generation Arduino, and display current state + statistics of network

#include <LiquidCrystal_I2C.h>

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
int lastPowerButtonState = LOW;
int lastGenerateButtonState = LOW;

// state variables
bool credentialsReceived = false;
int powerState = LOW; // LOW: off, HIGH: on
int userCount = 0; // number of users connected to network

// data to send to second Arduino
String SSID = "";
String password = "";

// basic initialization
void setup() {
  // get lcd ready for displaying
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // set up pin modes for input/output
  pinMode(offLight, OUTPUT);
  pinMode(onLight, OUTPUT);
  pinMode(powerButton, INPUT);
  pinMode(generateButton, INPUT);
}

// calls all helper functions
void loop() {
  if (millis() - prevMillis <= interval) {
    return;
  }

  if (!credentialsReceived) {
    getCredentials();
  }

  handlePowerButton();
  handleGenerateButton();

  updateUserCount();
}

// gathers credentials from Serial input and loads into global buffers
void getCredentials() {

}

// perform debounce check, then switch power of system and update led light status
void handlePowerButton() {
  int reading = digitalRead(powerButton);
  if (reading != lastPowerButtonState) {
    lastPowerDebounceTime = millis();
  }
  if (millis() - lastPowerDebounceTime <= debounceInterval) {
    return;
  }
  if (reading == powerState) {
    return;
  }

  powerState = reading;
  digitalWrite(!powerState, offLight);
  digitalWrite(powerState, onLight);

  lastPowerButtonState = reading;
}

// perform debounce check, then send data to QR code generator
void handleGenerateButton() {
  int reading = digitalRead(generateButton);
  if (reading != lastGenerateButtonState) {
    lastPowerDebounceTime = millis();
  }
  if (millis() - lastGenerateDebounceTime <= debounceInterval) {
    return;
  }

  sendData();
}

// send data to QR code generation Arduino
void sendData() {

}

// update the user count displayed on the lcd screen
void updateUserCount() {

}
