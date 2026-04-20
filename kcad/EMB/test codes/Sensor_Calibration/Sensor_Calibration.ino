#define SOIL_PIN 1

void setup() {
  Serial.begin(115200);

  analogReadResolution(12); // 0–4095
}

void loop() {
  int raw = analogRead(SOIL_PIN);

  Serial.print("RAW: ");
  Serial.println(raw);

  delay(1000);
}