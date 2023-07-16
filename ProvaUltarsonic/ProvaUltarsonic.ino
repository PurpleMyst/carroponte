#include <Ultrasonic.h>

#define PIEZO_PIN 6
bool status = LOW;

Ultrasonic sonar(10, 9);

void setup() {
  Serial.begin(9600);
  pinMode(PIEZO_PIN, OUTPUT);
}

void click() {
  if (status == LOW) {
    tone(PIEZO_PIN, 100, 50);
    status = HIGH;
  } else {
    noTone(PIEZO_PIN);
    status = LOW;
  }
}

void loop() {
  unsigned long sensorReading = sonar.read();
  Serial.println(sensorReading);
  click();
  delay(map(sensorReading, 5, 20, 100, 500));
}
