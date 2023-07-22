#include "ServoAxis.hpp"
#include "Zone.hpp"

const uint8_t ELECTROMAGNET_PIN = 24;
const uint8_t MANUAL_CONTROL_ENABLE_PIN = 30;
const uint8_t DESIRED_ELECTROMAGNET_STATE_PIN = 31;
const uint8_t DESIRED_X_PIN = A6;
const uint8_t DESIRED_Y_PIN = A7;
const uint8_t DESIRED_Z_PIN = A8;

const int16_t FIRST_ZONE_Y = 0;
const int16_t SECOND_ZONE_Y = 120;

const int16_t PORT_X = 100;
const int16_t BOAT_X = 0;

const int16_t DESIRE_THRESHOLD = 300;

const int16_t RETRACTION_Z = 80;
const int16_t EXTENSION_Z = 175;

const size_t BOAT_ZONES = 2;
const size_t PORT_ZONES = 2;

Zone boatZones[BOAT_ZONES] = {
    {A0, BOAT_X, FIRST_ZONE_Y},
    {A1, BOAT_X, SECOND_ZONE_Y},
};
Zone portZones[PORT_ZONES] = {
    {A2, PORT_X, FIRST_ZONE_Y},
    {A3, PORT_X, SECOND_ZONE_Y},
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

    x.attach(3, 0);
    y.attach(4, FIRST_ZONE_Y);
    z.attach(5, RETRACTION_Z);
    Serial.println("SETUP COMPLETE");
}

void moveContainer(Zone& src, Zone& dst) {
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

        for (Zone &dst : sources) {
            dst.updateState();
            if (dst.state != EMPTY)
                continue;

            moveContainer(src, dst);
            break;
        }
    }
}

void loop() {
    if (digitalRead(MANUAL_CONTROL_ENABLE_PIN) == LOW) {
        Serial.println("Manual control enabled, skipping automatic movement.");
        int16_t x = analogRead(DESIRED_X_PIN);
        int16_t y = analogRead(DESIRED_Y_PIN);
        bool zExtensionRequested = digitalRead(DESIRED_Z_PIN) == LOW;
        bool electromagnet = digitalRead(DESIRED_ELECTROMAGNET_STATE_PIN) == LOW;
        Serial.print("Moving to (");
        Serial.print(x);
        Serial.print(", ");
        Serial.print(y);
        Serial.print(", ");
        Serial.print(z);
        Serial.print(") with electromagnet ");
        Serial.println(electromagnet ? "enabled" : "disabled");
        x.moveTo(x > DESIRE_THRESHOLD ? PORT_X : BOAT_X);
        y.moveTo(y > DESIRE_THRESHOLD ? FIRST_ZONE_Y : SECOND_ZONE_Y);
        z.moveTo(zExtensionRequested ? EXTENSION_Z : RETRACTION_Z);
        setElectromagnet(electromagnet);
        return;
    }

    moveBetweenZones(boatZones, portZones);
    moveBetweenZones(portZones, boatZones);
}
