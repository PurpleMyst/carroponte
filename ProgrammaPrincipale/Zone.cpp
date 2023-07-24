#include "Zone.hpp"

const uint64_t CALIBRATION_TOTAL_TIME = 2000;
const uint64_t DETECTION_TIME = 500;
const double AMBIENT_LIGHT_FRACTION = 0.35;

Zone::Zone(uint8_t photoresistorPin, uint8_t plcPin, int16_t x, int16_t y)
    : photoresistorPin(photoresistorPin), plcPin(plcPin), x(x), y(y), detectionThreshold(0), state(EMPTY) {}

void Zone::calibrate() {
    double sum = 0;
    double readings = 0;

    for (uint64_t start = millis(); millis() < start + CALIBRATION_TOTAL_TIME;) {
        double value = analogRead(photoresistorPin);
        sum += value;
        readings += 1;
    }
    double mean = sum / readings;

    Serial.print("Zone @ (");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.print(") has mean of ");
    Serial.print(mean);
    Serial.print(" w/ threshold of ");
    Serial.println(AMBIENT_LIGHT_FRACTION * mean);
    detectionThreshold = AMBIENT_LIGHT_FRACTION * mean;
}

bool Zone::containerDetected() {
    double sum = 0;
    double readings = 0;

    for (uint64_t start = millis(); millis() < start + DETECTION_TIME;) {
        double value = analogRead(photoresistorPin);
        sum += value;
        readings += 1;
    }
    double mean = sum / readings;
    bool detection = mean <= detectionThreshold;
    pinMode(plcPin, OUTPUT);
    digitalWrite(plcPin, detection ? HIGH : LOW);
    return detection;
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
