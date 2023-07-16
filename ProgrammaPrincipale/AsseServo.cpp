#include "AsseServo.hpp"

AsseServo::AsseServo(uint8_t pin) { servo.attach(pin); }

void AsseServo::setDesiredPosition(uint8_t position) { desiredPosition = position; }

bool AsseServo::tick() {
    if (servo.read() < desiredPosition) {
        servo.write(servo.read() + 1);
        return false;
    } else if (servo.read() > desiredPosition) {
        servo.write(servo.read() - 1);
        return false;
    } else {
        return true;
    }
}
