#include <Servo.h>

Servo servo;

void setup() {
    servo.attach(6);
    servo.write(120);
    delay(1000 * 5);
    for (int i = servo.read(); i >= 25; i -= 5) {
      servo.write(i);
      delay(50);
    }
    
}

void loop() { /* meep */ }
