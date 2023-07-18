#include "ServoAxis.hpp"

void ServoAxis::attach(uint8_t pin, int16_t initialPosition) {
    servo.attach(pin);
    moveTo(initialPosition);
}

void ServoAxis::moveTo(int16_t desiredPosition) {
    int diff = servo.read() > desiredPosition ? -1 : 1;
    for (int16_t i = servo.read(); i != desiredPosition; i += diff) {
        servo.write(i);
        delay(7);
    }
    delay(50);
}
