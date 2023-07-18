#include "ServoAxis.hpp"

ServoAxis x, y, z;

void setup() {
    Serial.begin(9600);
    x.attach(3, 0);
    y.attach(4, 0);
    z.attach(5, 0);
}

void loop() {
    // Nothing to do here, we'll just wait for something to be available on the serial port.
}

void serialEvent() {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.startsWith("x")) {
        x.moveTo(line.substring(1).toInt());
    } else if (line.startsWith("y")) {
        y.moveTo(line.substring(1).toInt());
    } else if (line.startsWith("z")) {
        z.moveTo(line.substring(1).toInt());
    }
}
