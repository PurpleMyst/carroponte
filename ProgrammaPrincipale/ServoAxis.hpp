#include "Arduino.h"
#include "Servo.h"

class ServoAxis {
  private:
    Servo servo;

  public:
    void attach(uint8_t pin, int16_t initialPosition);

    void moveTo(int16_t desiredPosition);
};