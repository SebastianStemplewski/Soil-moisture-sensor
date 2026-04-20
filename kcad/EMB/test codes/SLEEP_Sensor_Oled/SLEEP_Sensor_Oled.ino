#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include "esp_sleep.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Preferences prefs;

// ENCODER
#define PIN_A 2
#define PIN_B 3
#define PIN_SW 4

// SENSOR
#define SOIL_PIN 1

int lastStateA;
int humSet;

bool editMode = false;
bool blinkState = true;

unsigned long lastBlink = 0;
unsigned long lastActivity = 0;

// ----------- ODCZYT -----------

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

  if (editMode) {
    if (millis() - lastBlink > 400) {
      blinkState = !blinkState;
      lastBlink = millis();
    }
  } else {
    blinkState = true;
  }

  display.setCursor(0, 0);
  display.print("Sensor: ");
  display.print(humidity);
  display.print("%");

  display.setCursor(0, 16);

  if (blinkState) display.print("Set: ");
  else display.print("     ");

  display.print(humSet);
  display.print("%");

  display.display();
}

// ----------- SLEEP -----------

void goToSleep() {
  prefs.putInt("humSet", humSet);

  esp_sleep_enable_timer_wakeup(60 * 1000000); // 1 minuta

  esp_sleep_enable_ext1_wakeup(
    1ULL << PIN_SW,
    ESP_EXT1_WAKEUP_ANY_LOW
  );

  esp_deep_sleep_start();
}

// ----------- SETUP -----------

void setup() {
  Serial.begin(115200);

  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_SW, INPUT_PULLUP);

  analogReadResolution(12);

  prefs.begin("settings", false);
  humSet = prefs.getInt("humSet", 50);

  esp_sleep_wakeup_cause_t wakeup = esp_sleep_get_wakeup_cause();

  // Jeśli wybudzony timerem → tylko pomiar
  if (wakeup == ESP_SLEEP_WAKEUP_TIMER) {
    int humidity = readHumidity();

    Serial.print("Pomiar: ");
    Serial.println(humidity);

    delay(100);
    goToSleep();
  }

  // NORMALNY TRYB (po kliknięciu)
  Wire.begin(6, 7);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  lastStateA = digitalRead(PIN_A);
  lastActivity = millis();
}

// ----------- LOOP -----------

void loop() {
  int humidity = readHumidity();

  int currentStateA = digitalRead(PIN_A);

  // ENCODER
  if (currentStateA != lastStateA) {
    lastActivity = millis();

    if (editMode) {
      if (digitalRead(PIN_B) != currentStateA) humSet++;
      else humSet--;

      humSet = constrain(humSet, 0, 100);
    }
  }

  lastStateA = currentStateA;

  // PRZYCISK
  if (digitalRead(PIN_SW) == LOW) {
    delay(200);
    editMode = !editMode;
    lastActivity = millis();
  }

  drawScreen(humidity);

  // AUTO SLEEP po 10s
  if (millis() - lastActivity > 10000) {
    goToSleep();
  }

  delay(50);
}