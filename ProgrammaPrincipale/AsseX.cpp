#include "AsseX.hpp"

namespace XAxis {

bool checkBoost(double currentPos) {
    if (lastBoostCheckTime == -1) {
        lastBoostCheckTime = millis();
        lastBoostCheckPos = currentPos;
        return false;
    }
    if ((millis() - lastBoostCheckTime) <
        BOOST_CHECK_INTERVAL) {
        return false;
    }
    lastBoostCheckTime = millis();
    if (abs(currentPos - lastBoostCheckPos) <
        BOOST_CHECK_TOLERANCE) {
        return false;
    }
    lastBoostCheckPos = currentPos;
    return true;
}

void resetBoost() { lastBoostCheckTime = -1; }

void setSpeed(uint8_t speed) {
  Serial.print("writing to ");
  Serial.print(SPEED_PIN);
  Serial.print(" a speed of ");
  Serial.println(speed);
    analogWrite(SPEED_PIN, (int) speed);
}

void forward() {
    setSpeed(DEFAULT_SPEED);
    digitalWrite(D1_PIN, LOW);
    digitalWrite(D2_PIN, HIGH);
}

void backward() {
    setSpeed(DEFAULT_SPEED);
    digitalWrite(D1_PIN, HIGH);
    digitalWrite(D2_PIN, LOW);
}

void stop() {
    setSpeed(0xFF);
    digitalWrite(D1_PIN, LOW);
    digitalWrite(D2_PIN, LOW);
    resetBoost();
}

double actualPosition() {
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
    return ((double)pulseIn(ECHO_PIN, HIGH)) /
           58.3;
}

void moveTo(double desiredPosition) {
    double currentPos = actualPosition();
        if (currentPos < desiredPosition) {
        Serial.println(" FORWARD");
        forward();
    } else {
      Serial.println(" BACKWARD");
        backward();
    }
    setSpeed(BOOST_SPEED);
    delay(BOOST_AMOUNT);
    setSpeed(DEFAULT_SPEED);

  for (;;) {
    double currentPos = actualPosition();
    Serial.print(currentPos);
    Serial.print(" => ");
    Serial.print(desiredPosition);

    double error =
        abs(currentPos - desiredPosition);
    if (error <= ERROR_TOLERANCE) {
        stop();
        return;
    }

    /*if (checkBoost(currentPos)) {
        setSpeed(BOOST_SPEED);
        delay(BOOST_AMOUNT);
        setSpeed(DEFAULT_SPEED);
    }*/

    if (currentPos < desiredPosition) {
        Serial.println(" FORWARD");
        forward();
    } else {
      Serial.println(" BACKWARD");
        backward();
    }
}}

}  // namespace XAxis
