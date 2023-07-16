#include <Servo.h>

#include "Arduino.h"

class AsseServo {
   private:
    Servo servo;
    uint8_t desiredPosition;

   public:
    AsseServo(uint8_t pin);

    void setDesiredPosition(uint8_t position);

    bool tick();
};
