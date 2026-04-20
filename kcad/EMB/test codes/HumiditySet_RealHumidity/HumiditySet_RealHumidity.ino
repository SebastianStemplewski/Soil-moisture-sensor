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

// SOIL SENSOR
#define SOIL_PIN 1

int lastStateA;
int humSet = 50;

bool editMode = false;
bool blinkState = true;

unsigned long lastBlink = 0;

// ----------- ODCZYT WILGOTNOŚCI (STABILNY) -----------

int readHumidity() {
  long sum = 0;

  for (int i = 0; i < 10; i++) {
    sum += analogRead(SOIL_PIN);
    delay(5);
  }

  int raw = sum / 10;

  int humidity = map(raw, 2620, 920, 0, 100);

  return constrain(humidity, 0, 100);
}

// ----------- OLED -----------

void drawScreen(int humidity) {
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

  // GÓRA
  display.setCursor(0, 0);
  display.print("Sensor: ");
  display.print(humidity);
  display.print("%");

  // DÓŁ
  display.setCursor(0, 16);

  if (blinkState) {
    display.print("Set: ");
  } else {
    display.print("     "); // znika tekst dla efektu mrugania
  }

  display.print(humSet);
  display.print("%");

  display.display();
}

// ----------- SETUP -----------

void setup() {
  Serial.begin(115200);

  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_SW, INPUT_PULLUP);

  lastStateA = digitalRead(PIN_A);

  analogReadResolution(12);

  Wire.begin(6, 7);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAIL");
    while (true);
  }

  display.clearDisplay();
}

// ----------- LOOP -----------

void loop() {
  int humidity = readHumidity();

  int currentStateA = digitalRead(PIN_A);

  // ENCODER
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

  drawScreen(humidity);

  delay(50);
}