#pragma once

#include "Arduino.h"

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
    int16_t detectionThreshold;

    Zone(uint8_t, int16_t, int16_t, int16_t);

    bool containerDetected();
    void updateState();
};
