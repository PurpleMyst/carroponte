#include "ServoAxis.hpp"
#include "Zone.hpp"

const uint8_t ELECTROMAGNET_PIN = 48;
const uint8_t MANUAL_CONTROL_ENABLE_PIN = 50;
const uint8_t DESIRED_ELECTROMAGNET_STATE_PIN = 51;
const uint8_t DESIRED_X_PIN = A6;
const uint8_t DESIRED_Y_PIN = A7;
const uint8_t DESIRED_Z_PIN = A8;

const int16_t FIRST_ZONE_Y = 0;
const int16_t SECOND_ZONE_Y = 110;

const int16_t PORT_X = 100;
const int16_t BOAT_X = 0;

const int16_t DESIRE_THRESHOLD = 300;

const int16_t RETRACTION_Z = 0;
const int16_t EXTENSION_Z = 105;

const size_t BOAT_ZONES = 2;
const size_t PORT_ZONES = 2;

Zone boatZones[BOAT_ZONES] = {
    {A15, 46, BOAT_X, FIRST_ZONE_Y},
    {A14, 44, BOAT_X, SECOND_ZONE_Y},
};
Zone portZones[PORT_ZONES] = {
    {A13, 42, PORT_X, FIRST_ZONE_Y},
    {A12, 40, PORT_X, SECOND_ZONE_Y},
};

ServoAxis x, y, z;

bool electromagnetEnabled = false;

void setElectromagnet(bool enabled) {
    electromagnetEnabled = enabled;
    digitalWrite(ELECTROMAGNET_PIN, enabled ? HIGH : LOW);
}

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        delay(1);
    };

    pinMode(ELECTROMAGNET_PIN, OUTPUT);
    pinMode(MANUAL_CONTROL_ENABLE_PIN, INPUT_PULLUP);
    pinMode(DESIRED_ELECTROMAGNET_STATE_PIN, INPUT_PULLUP);
    pinMode(DESIRED_Z_PIN, INPUT_PULLUP);

    Serial.println("HOMING AXES");
    x.attach(3, 6, 0);
    y.attach(4, 7, FIRST_ZONE_Y);
    z.attach(5, 8, RETRACTION_Z);

    Serial.println("CALIBRATING ZONES (TAKES A WHILE)");
    for (Zone &zone : boatZones) {
        zone.calibrate();
    }
    for (Zone &zone : portZones) {
        zone.calibrate();
    }

    Serial.println("SETUP COMPLETE");
}

void moveContainer(Zone &src, Zone &dst) {
    // Print out what we're going to do.
    Serial.print("Moving container from (");
    Serial.print(src.x);
    Serial.print(", ");
    Serial.print(src.y);
    Serial.print(") to (");
    Serial.print(dst.x);
    Serial.print(", ");
    Serial.print(dst.y);
    Serial.println(")!");

    // Execute move sequence.
    x.moveTo(src.x);
    y.moveTo(src.y);

    z.moveTo(EXTENSION_Z);
    setElectromagnet(true);
    z.moveTo(RETRACTION_Z);

    x.moveTo(dst.x);
    y.moveTo(dst.y);

    z.moveTo(EXTENSION_Z);
    setElectromagnet(false);
    z.moveTo(RETRACTION_Z);

    delay(1000);

    // Check if movement actually happened, and if so update the states.
    if (!src.containerDetected()) {
        Serial.println("Container not detected at source zone, setting state to EMPTY.");
        src.state = EMPTY;
    } else {
        Serial.println("Container detected at source zone, something must've gone wrong!");
    }
    if (dst.containerDetected()) {
        Serial.println("Container detected at destination zone, setting state to MOVED_TO.");
        dst.state = MOVED_TO;
    } else {
        Serial.println("Container not detected at destination zone, something must've gone wrong!");
    }
}

template <size_t N, size_t M> void moveBetweenZones(Zone (&sources)[N], Zone (&destinations)[M]) {
    for (Zone &src : sources) {
        src.updateState();
        if (src.state != HAS_CONTAINER_TO_MOVE)
            continue;

        for (Zone &dst : destinations) {
            dst.updateState();
            if (dst.state != EMPTY)
                continue;

            moveContainer(src, dst);
            break;
        }
    }
}

void manualControl() {
    Serial.println("Manual control enabled, skipping automatic movement.");
    int16_t xDesired = analogRead(DESIRED_X_PIN);
    int16_t yDesired = analogRead(DESIRED_Y_PIN);
    bool zExtensionRequested = digitalRead(DESIRED_Z_PIN) == HIGH;
    bool electromagnet = digitalRead(DESIRED_ELECTROMAGNET_STATE_PIN) == LOW;
    Serial.print("Moving to (");
    Serial.print(xDesired);
    Serial.print(", ");
    Serial.print(yDesired);
    Serial.print(", ");
    Serial.print(zExtensionRequested ? "extended" : "retracted");
    Serial.print(") with electromagnet ");
    Serial.println(electromagnet ? "enabled" : "disabled");
    x.moveTo(xDesired > DESIRE_THRESHOLD ? PORT_X : BOAT_X);
    y.moveTo(yDesired > DESIRE_THRESHOLD ? FIRST_ZONE_Y : SECOND_ZONE_Y);
    z.moveTo(zExtensionRequested ? EXTENSION_Z : RETRACTION_Z);
    setElectromagnet(electromagnet);
}

void loop() {
    if (digitalRead(MANUAL_CONTROL_ENABLE_PIN) == LOW) {
        manualControl();
        return;
    }

    moveBetweenZones(boatZones, portZones);
    moveBetweenZones(portZones, boatZones);
}
