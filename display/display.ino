// Logan Lucas, Henry Chen, Lance Guevarra
// netids: lluca5, hchen250, lguev2
// UINs: 667695865, 675227500, 657508441

// Display Arduino
// Description: Get parsed QR code string from generator Arduino, then draw to OLED display. Also, update number of users from button press to the network manager Arduino.

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <qrcode.h>

// set up third arduino Serial connection
const int rxPin = 2;
const int txPin = 3;

SoftwareSerial generatorArduino(rxPin, txPin);

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

const int potentiometer = A0;
const int buzzer = 8;
const int countButton = 7;

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

char wifiBuffer[128];

bool dataReceived = false;
bool isBuzzerActive = false;

// data for the potentiometer to adjust the buzzer frequency
int buzzerFrequency = 200;
const int minFrequency = 100;
const int maxFrequency = 2000;

unsigned long buzzerStartTime = 0;

const int debounceInterval = 50;
unsigned long lastDebounceTime = 0;
int buttonState = LOW;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  generatorArduino.begin(9600);

  pinMode(buzzer, OUTPUT);
  pinMode(countButton, INPUT);
  pinMode(potentiometer, INPUT);

  // initialize OLED screen
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display(); // display blank screen so that previous iterations don't hang
}

void loop() {
  updateBuzzerFrequency(); // checks potentiometer value

  int reading = digitalRead(countButton);

  if (reading != buttonState) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceInterval) {
    if (reading != buttonState) {
      buttonState = reading;
    }
  }

  // sends a signal to the network manager Arduino to show that a new user has connected
  if (reading == HIGH && reading != buttonState) {
    Serial1.write("U");
    tone(buzzer, buzzerFrequency);
    buzzerStartTime = millis();
    isBuzzerActive = true;
  }
  
  // stop buzzer after 200 milliseconds
  if (isBuzzerActive && millis() - buzzerStartTime >= 200) {
    noTone(buzzer);
    isBuzzerActive = false;
  }

  buttonState = reading;

  // get string from generator Arduino
  if (generatorArduino.available() > 0 && !dataReceived) {
    int n = generatorArduino.readBytesUntil('\n', wifiBuffer, sizeof(wifiBuffer) - 1);
    wifiBuffer[n] = '\0';

    Serial.print("received data: ");
    Serial.println(wifiBuffer);

    dataReceived = true;

    generateQRCode(wifiBuffer);
  }
}

// draws data to the screen based on the 'text' parameter
void generateQRCode(const char* text) {
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, text);

  display.clearDisplay();

  int scale = min(OLED_WIDTH / qrcode.size, OLED_HEIGHT / qrcode.size);

  int shiftX = (OLED_WIDTH - qrcode.size * scale) / 2;

  int shiftY = (OLED_HEIGHT - qrcode.size * scale) / 2;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(shiftX + x * scale, shiftY + y * scale, scale, scale, WHITE);
      }
    }
  }

  display.display();
}

// gets data from potentiometer and adjusts frequency based on results
void updateBuzzerFrequency() {
  int reading = analogRead(potentiometer);
  buzzerFrequency = map(reading, 0, 1023, minFrequency, maxFrequency);
}
