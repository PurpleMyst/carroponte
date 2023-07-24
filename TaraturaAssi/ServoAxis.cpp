#include "ServoAxis.hpp"

void ServoAxis::attach(uint8_t servoPin, uint8_t commsPin, int16_t initialPosition)  {
    this->commsPin = commsPin;
    pinMode(commsPin, OUTPUT);
      servo.attach(servoPin);
    moveTo(initialPosition);
}

void ServoAxis::moveTo(int16_t desiredPosition) {
    int16_t delta = servo.read() > desiredPosition ? -1 : 1;
    for (int16_t angle = servo.read(); angle != desiredPosition; angle += delta) {
        analogWrite(commsPin, angle);
        servo.write(angle);
        delay(40);
    }
    delay(50);
}
