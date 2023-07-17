#include <Servo.h>

#include "Arduino.h"

class AsseServo {
private:
    Servo servo;
public:
    void attach(uint8_t pin, int16_t initialPosition);

    void moveTo(int16_t desiredPosition);
};
