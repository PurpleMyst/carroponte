#pragma once
#include "Arduino.h"

namespace XAxis {

const uint8_t SPEED_PIN = 11;
const uint8_t D1_PIN = 10;
const uint8_t D2_PIN = 12;
const uint8_t POSITIONS = 3;

const uint8_t TRIGGER_PIN = 7;
const uint8_t ECHO_PIN = 8;
const double ERROR_TOLERANCE = 0.15;

const uint8_t DEFAULT_SPEED = 150;

const uint8_t BOOST_SPEED = 255;
const uint8_t BOOST_AMOUNT = 25;
const uint8_t BOOST_CHECK_INTERVAL = 1000;
const uint8_t BOOST_CHECK_TOLERANCE = 0.5;

const double stops[POSITIONS] = {10, 15.4,
                                 18.5};
const double homePosition = 7;

static int64_t lastBoostCheckTime = 0;
static double lastBoostCheckPos = 0;

bool checkBoost(double currentPos);

void resetBoost();

void setSpeed(uint8_t speed);

void forward();

void backward();

void stop();

double actualPosition();

void moveTo(double position);

}  // namespace XAxis
