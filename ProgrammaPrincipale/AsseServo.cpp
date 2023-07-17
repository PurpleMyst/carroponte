#include "AsseServo.hpp"

void AsseServo::attach(uint8_t pin, int16_t initialPosition) {
  servo.attach(pin);
  moveTo(initialPosition);
}

void AsseServo::moveTo(int16_t desiredPosition) {
  int diff = servo.read() > desiredPosition ? -1 : 1;
  for (int i = servo.read(); i != desiredPosition; i += diff) {
    servo.write(i);
    delay(7);
  }
  delay(50);
}
