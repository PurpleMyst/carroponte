#include "ServoAxis.hpp"

ServoAxis x, y, z;

const uint8_t MANUAL_CONTROL_ENABLE_PIN = 30;
const uint8_t DESIRED_X_PIN             = A6;
const uint8_t DESIRED_Y_PIN             = A7;
const uint8_t DESIRED_Z_PIN             = A8;

void setup() {
    Serial.begin(9600);
    x.attach(3, 6, 0);
    y.attach(4, 7, 0);
    z.attach(5, 8, 0);
    pinMode(MANUAL_CONTROL_ENABLE_PIN, INPUT_PULLUP);
}

void loop() {
    if (digitalRead(MANUAL_CONTROL_ENABLE_PIN) == LOW) {
        Serial.print("Reading manual control: ");
        Serial.print(analogRead(DESIRED_X_PIN));
        Serial.print(' ');
        Serial.print(analogRead(DESIRED_Y_PIN));
        Serial.print(' ');
        Serial.println(analogRead(DESIRED_Z_PIN));
        delay(1000);
    }
}

void serialEvent() {
    String line = Serial.readStringUntil('\n');
    line.trim();
    String angleString = line.substring(1);
    angleString.trim();
    int16_t angle = angleString.toInt();
    if (line.startsWith("x")) {
        Serial.print("Moving x to ");
        Serial.println(angle);
        x.moveTo(angle);
    } else if (line.startsWith("y")) {
        Serial.print("Moving y to ");
        Serial.println(angle);
        y.moveTo(angle);
    } else if (line.startsWith("z")) {
        Serial.print("Moving z to ");
        Serial.println(angle);
        z.moveTo(angle);
    }
}
