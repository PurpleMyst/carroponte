#include "Arduino.h"
#include "Servo.h"

class ServoAxis {
  private:
    Servo servo;
    uint8_t commsPin;

  public:
    void attach(uint8_t servoPin, uint8_t commsPin, int16_t initialPosition);

    void moveTo(int16_t desiredPosition);
};
