#include "Zone.hpp"

Zone::Zone(uint8_t pin, int16_t x, int16_t y) : photoresistorPin(pin), x(x), y(y), state(EMPTY) {}

bool Zone::containerDetected() { return analogRead(photoresistorPin) <= PHOTO_THRESHOLD; }

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
