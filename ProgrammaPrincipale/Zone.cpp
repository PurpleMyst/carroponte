#include "Zone.hpp"

bool containerDetected(uint8_t pin) { return analogRead(pin) <= PHOTO_THRESHOLD; }

Zone::Zone(uint8_t pin, int16_t x, int16_t y) : photoresistorPin(pin), x(x), y(y) {
    state = containerDetected(pin) ? HAS_CONTAINER_TO_MOVE : EMPTY;
}

bool Zone::containerDetected() { return containerDetected(photoresistorPin); }

void updateState() {
    bool hasContainer = containerDetected();
    if (state != EMPTY && !hasContainer) {
        Serial.print("Container not detected anymore at (");
        Serial.print(x);
        Serial.print(", ");
        Serial.print(x);
        Serial.println(").");
        state = EMPTY;
    } else if (state == EMPTY && hasContainer) {
        Serial.print("Container to move detected at (");
        Serial.print(x);
        Serial.print(", ");
        Serial.print(x);
        Serial.println(").");
        state = HAS_CONTAINER_TO_MOVE;
    }
}
