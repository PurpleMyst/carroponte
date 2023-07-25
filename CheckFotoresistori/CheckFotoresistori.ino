#include "Arduino.h"

uint8_t pins[4] = {A12, A13, A14, A15};
int16_t thresholds[4] = {150, 50, 170, 125};

void setup() {
    Serial.begin(9600);
}

void loop() {
    for (int i = 0; i < 4; i++) {
        if (i != 0) Serial.print(' ');
        Serial.print(analogRead(pins[i]));
    }
    Serial.println();
}
