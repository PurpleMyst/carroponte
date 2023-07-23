#pragma once

#include "Arduino.h"

const double Tf = 0.01; // Time costant of the low-pass filter for light detection. Can tune!

const uint64_t LIGHT_CALIBRATION_TIME = 5000;
const uint64_t UPDATE_INTERVAL = 200; /* ms */

enum ZoneState {
    // This zone is empty and ready to receive a container.
    EMPTY,

    // We've moved a container to this zone that will need to be manually unloaded.
    MOVED_TO,

    // This zone holds a container that is ready to be moved.
    HAS_CONTAINER_TO_MOVE,
};

struct Zone {
    uint8_t photoresistorPin;

    enum ZoneState state;

    int16_t x, y;

    volatile double prevValue;
    volatile uint64_t prevTimestamp;

    double detectionThreshold;

    Zone(uint8_t, int16_t, int16_t);

    // Is a container in the zone?
    bool containerDetected();

    // Take a sample from the photoresistor and update the state.
    void updateMeasurement();

    // Update the state of the zone, i.e. if it has a container to move or if its now empty.
    void updateState();

    // Calibrate the ambient light level for this zone.
    void calibrateLight();
};
