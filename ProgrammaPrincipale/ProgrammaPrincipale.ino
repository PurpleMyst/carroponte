#include "AsseServo.hpp"
#include "AsseX.hpp"

const uint8_t ELECTROMAGNET_PIN = 13;

const uint8_t PHOTO_THRESHOLD = 30;

struct Photoresistor {
    uint8_t pin;

    double x;
    uint8_t y;

    bool containerDetected() {
        return analogRead(pin) <=
               PHOTO_THRESHOLD;
    }
};

const size_t BOAT_PHOTORESISTORS = 2;
const size_t PORT_PHOTORESISTORS_FOR_UNLOADING =
    1;
const size_t PORT_PHOTORESISTORS_FOR_LOADING =
    1;

const int FIRST_ZONE_Y = 0;
const int SECOND_ZONE_Y = 120;

const uint8_t RETRACTION_Z = 80;
const uint8_t EXTENSION_Z = 175;

Photoresistor boatSrc {A1, 10, FIRST_ZONE_Y};
Photoresistor boatDst {A0, 10, SECOND_ZONE_Y};

Photoresistor portDst
        {A3, 15.4, FIRST_ZONE_Y
};
Photoresistor portSrc
        {A2, 15.4, SECOND_ZONE_Y};


byte mac[] = {0xE0, 0xDC, 0xA0,
              0x14, 0x07, 0x80};
byte ip[] = {192, 168, 1, 151};

AsseServo y;

AsseServo z;

bool electromagnetEnabled = false;

void setup() {
    Serial.begin(9600);
    while (!Serial) {};

    pinMode(XAxis::SPEED_PIN, OUTPUT);
    pinMode(XAxis::D1_PIN, OUTPUT);
    pinMode(XAxis::D2_PIN, OUTPUT);
    pinMode(ELECTROMAGNET_PIN, OUTPUT);

    y.attach(5, FIRST_ZONE_Y);
    z.attach(6, RETRACTION_Z);
    Serial.println("CALIBRATION COMPLETE");
}

void setElectromagnet(bool enabled) {
  
    electromagnetEnabled = enabled;
    digitalWrite(ELECTROMAGNET_PIN,
                 enabled ? HIGH : LOW);
}

void moveContainer(Photoresistor src,
                   Photoresistor dst) {
  Serial.print(src.x);
    Serial.print(' ');
    Serial.print(src.y);
    Serial.print(" => ");
    Serial.print(dst.x);
    Serial.print(' ');
    Serial.println(dst.y);
    
    XAxis::moveTo(src.x);
    Serial.println("got into x");
    y.moveTo(src.y);
    Serial.println("got into y;");

    z.moveTo(EXTENSION_Z);
    Serial.println("extended z");
    setElectromagnet(true);
    z.moveTo(RETRACTION_Z);
    Serial.println("retracted z");

    XAxis::moveTo(dst.x);
    y.moveTo(dst.y);

    z.moveTo(EXTENSION_Z);
    setElectromagnet(false);
    z.moveTo(RETRACTION_Z);
}

void loop() {

    if (portSrc.containerDetected() && !boatDst.containerDetected()) moveContainer(portSrc, boatDst);
    if (boatSrc.containerDetected() && !portDst.containerDetected()) moveContainer(boatSrc, portDst);

}
