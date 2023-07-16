#include "AsseServo.hpp"
#include "AsseX.hpp"

const uint8_t ELECTROMAGNET_PIN = 13;

const uint8_t PHOTO_THRESHOLD = 100;

struct Photoresistor {
    uint8_t pin;

    double x;
    uint8_t y;

    bool containerDetected() { return analogRead(pin) >= PHOTO_THRESHOLD; }
};

const size_t BOAT_PHOTORESISTORS = 2;
const size_t PORT_PHOTORESISTORS_FOR_UNLOADING = 2;
const size_t PORT_PHOTORESISTORS_FOR_LOADING = 2;

const uint8_t FIRST_ZONE_Y = 0;
const uint8_t SECOND_ZONE_Y = 0;

const uint8_t RETRACTION_Z = 0;
const uint8_t EXTENSION_Z = 0;

Photoresistor boatPhotoresistors[BOAT_PHOTORESISTORS] = {
    {A0, 0, FIRST_ZONE_Y},
    {A1, 0, SECOND_ZONE_Y},
};
Photoresistor portPhotoresistorsForUnloading[PORT_PHOTORESISTORS_FOR_UNLOADING] = {
    {A2, 1, FIRST_ZONE_Y},
    {A3, 1, SECOND_ZONE_Y},
};
Photoresistor portPhotoresistorsForLoading[PORT_PHOTORESISTORS_FOR_LOADING] = {
    {A4, 2, FIRST_ZONE_Y},
    {A5, 2, SECOND_ZONE_Y},
};

byte mac[] = {0xE0, 0xDC, 0xA0, 0x14, 0x07, 0x80};
byte ip[] = {192, 168, 1, 151};

AsseServo y(0);

AsseServo z(0);

bool electromagnetEnabled = false;

void setup() {
    Serial.begin(9600);

    pinMode(XAxis::SPEED_PIN, OUTPUT);
    pinMode(XAxis::D1_PIN, OUTPUT);
    pinMode(XAxis::D2_PIN, OUTPUT);
    pinMode(ELECTROMAGNET_PIN, OUTPUT);
}

void setElectromagnet(bool enabled) {
    electromagnetEnabled = enabled;
    digitalWrite(ELECTROMAGNET_PIN, enabled ? HIGH : LOW);
}

void moveContainer(Photoresistor src, Photoresistor dst) {
    XAxis::desiredPosition = src.x;
    while (!XAxis::tick()) { /* wait */ }
    y.setDesiredPosition(src.y);
    while (!y.tick()) { /* wait */ }

    z.setDesiredPosition(EXTENSION_Z);
    while (!z.tick()) { /* wait */ }
    setElectromagnet(true);
    z.setDesiredPosition(RETRACTION_Z);
    while (!z.tick()) { /* wait */ }

    XAxis::desiredPosition = dst.x;
    while (!XAxis::tick()) { /* wait */ }
    y.setDesiredPosition(dst.y);
    while (!y.tick()) { /* wait */ }

    z.setDesiredPosition(EXTENSION_Z);
    while (!z.tick()) { /* wait */ }
    setElectromagnet(false);
    z.setDesiredPosition(RETRACTION_Z);
    while (!z.tick()) { /* wait */ }
}

void loop() {
    for (int i = 0; i < BOAT_PHOTORESISTORS; i++) {
        if (!boatPhotoresistors[i].containerDetected()) continue;

        for (int j = 0; j < PORT_PHOTORESISTORS_FOR_UNLOADING; j++) {
            if (portPhotoresistorsForUnloading[j].containerDetected()) continue;

            moveContainer(boatPhotoresistors[i], portPhotoresistorsForUnloading[j]);
            return;
        }

        // Se arriviamo qui, non avevamo dove lasciare il container. Andiamo avanti.
        break;
    }

    for (int i = 0; i < PORT_PHOTORESISTORS_FOR_LOADING; i++) {
        if (!portPhotoresistorsForLoading[i].containerDetected()) continue;

        for (int j = 0; j < BOAT_PHOTORESISTORS; j++) {
            if (boatPhotoresistors[j].containerDetected()) continue;

            moveContainer(portPhotoresistorsForLoading[i], boatPhotoresistors[j]);
            return;
        }

        // Se arriviamo qui, non avevamo dove lasciare il container. Andiamo avanti.
        break;
    }
}
