#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ENCODER
#define PIN_A 2
#define PIN_B 3
#define PIN_SW 4

int lastStateA;
int humSet = 50;

bool editMode = false;
bool blinkState = true;

unsigned long lastBlink = 0;

// ----------- OLED -----------

void drawScreen() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // MRUGANIE "SET"
  if (editMode) {
    if (millis() - lastBlink > 400) {
      blinkState = !blinkState;
      lastBlink = millis();
    }
  } else {
    blinkState = true;
  }

  if (blinkState) {
    display.setCursor(0, 0);
    display.print("SET Humidity:");
  }

  display.setCursor(0, 16);
  display.print(humSet);
  display.print("%");

  display.display();
}

// ----------- SETUP -----------

void setup() {
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_SW, INPUT_PULLUP);

  lastStateA = digitalRead(PIN_A);

  Wire.begin(6, 7);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  display.clearDisplay();
}

// ----------- LOOP -----------

void loop() {
  int currentStateA = digitalRead(PIN_A);

  // ENCODER działa tylko w trybie edycji
  if (currentStateA != lastStateA) {
    if (editMode) {
      if (digitalRead(PIN_B) != currentStateA) {
        humSet++;
      } else {
        humSet--;
      }

      humSet = constrain(humSet, 0, 100);
    }
  }

  lastStateA = currentStateA;

  // PRZYCISK
  if (digitalRead(PIN_SW) == LOW) {
    delay(200);

    editMode = !editMode;
  }

  drawScreen();

  delay(10);
}