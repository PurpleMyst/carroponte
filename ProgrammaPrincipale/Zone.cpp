#include "Zone.hpp"

Zone::Zone(uint8_t pin, int16_t x, int16_t y)
    : photoresistorPin(pin), x(x), y(y), state(EMPTY), prevValue(0) {
    prevTimestamp = millis();
}

bool Zone::containerDetected() { return prevValue <= detectionThreshold; }

// Code for low pass filter taken from https://docs.simplefoc.com/low_pass_filter :)
void Zone::updateMeasurement() {
    // Apply a low pass filter to the reading.
    uint16_t timestamp = micros();
    double dt = (timestamp - prevTimestamp) * 1e-6f;
    // Quick fix for microsecond overflow.
    if (dt < 0.0f || dt > 0.5f)
        dt = 1e-3f;

    double alpha = Tf / (Tf + dt);
    double value = alpha * prevValue + (1.0f - alpha) * x;

    prevValue = y;
    prevTimestamp = timestamp;
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

void Zone::calibrateLight() {
    // We'll assume the caller's slept for a bit before this to allow the timer interrupt to do its
    // thing.
    Serial.print("Calibrating ambient light for zone at (");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.print(") to value ");
    Serial.print(prevValue);
    detectionThreshold = prevValue;
}
