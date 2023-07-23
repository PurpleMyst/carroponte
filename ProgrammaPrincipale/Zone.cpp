#include "Zone.hpp"

Zone::Zone(uint8_t pin, int16_t x, int16_t y, int16_t detectionThreshold) : photoresistorPin(pin), x(x), y(y), detectionThreshold(detectionThreshold), state(EMPTY) {}

bool Zone::containerDetected() { 
    int16_t value = analogRead(photoresistorPin);
    return value <= detectionThreshold; 
}

void Zone::updateState() {
    bool hasContainer = containerDetected();
    if (state != EMPTY && !hasContainer) {
        Serial.print("Container not detected anymore at (");
        Serial.print(x);
        Serial.print(", ");
        Serial.print(y);
        Serial.println(").");
        state = EMPTY;
    } else if (state == EMPTY && hasContainer) {
        Serial.print("Container to move detected at (");
        Serial.print(x);
        Serial.print(", ");
        Serial.print(y);
        Serial.println(").");
        state = HAS_CONTAINER_TO_MOVE;
    }
}
