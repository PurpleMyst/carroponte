#include <Servo.h>

Servo y;
Servo z;

#define Y_RETRACT -10
#define Y_EXTEND 120
#define Z_RETRACT 80
#define Z_EXTEND 185

void setup() {
  y.attach(5);
  z.attach(6);
  pinMode(13, OUTPUT);
  moveServo(y, Y_RETRACT);
  moveServo(z, Z_RETRACT);
}

void loop() {
  moveServo(y, Y_RETRACT);
  delay(3000);
  moveServo(y, Y_EXTEND);
  delay(3000);
  moveServo(z, Z_EXTEND);
  delay(1000);
  digitalWrite(13, HIGH);
  delay(1000);
  moveServo(z, Z_RETRACT);
  delay(1000);
  moveServo(z, Z_EXTEND);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
  moveServo(z, Z_RETRACT);
  delay(1000);
}

void moveServo(Servo s, int angle) {
  int diff = s.read() > angle ? -1 : 1;
  for (int i = s.read(); i != angle; i += diff) {
    s.write(i);
    delay(5);
  }
}
