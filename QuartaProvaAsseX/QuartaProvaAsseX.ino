#define MOTOR_SPEED_PIN       9
#define MOTOR_DIRECTION_PIN_1 10
#define MOTOR_DIRECTION_PIN_2 11
#define MOTOR_SPEED           180
#define POSITIONS             4

int positionPins[POSITIONS] = {2, 3, 4, 5};
int currentPosition = 0;

void setup() {
    // initialize serial communication
    Serial.begin(9600);

    // configure pins
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(MOTOR_SPEED_PIN, OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN_1, OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN_2, OUTPUT);
    for (int i = 0; i < POSITIONS; i++) {
        pinMode(positionPins[i], INPUT_PULLUP);
    }

    blink(10);

    // go home
    goBackward();
    while (digitalRead(positionPins[0]) != HIGH) {
        delay(1);
    }
    stopMotor();

    // test each position
    for (int i = 0; i < POSITIONS; i++) {
        Serial.print("Calibration for position:");
        Serial.println(i);
        blink(i + 1);
        goForward();
        while (digitalRead(positionPins[i]) != HIGH) {
            delay(1);
        }
        stopMotor();
        delay(1000);
    }

    // go home again and begin loop() with random positions
    goBackward();
    while (digitalRead(positionPins[0]) != HIGH) {
        delay(1);
    }

    // set random seed
    randomSeed(analogRead(0));
}

void loop() {
    int nextPosition = currentPosition;
    while (nextPosition == currentPosition) {
        nextPosition = random(0, POSITIONS);
    }
    Serial.print("Next position: ");
    Serial.println(nextPosition);

    blink(nextPosition + 1);
    if (nextPosition > currentPosition) {
        goForward();
    } else {
        goBackward();
    }
    while (digitalRead(positionPins[nextPosition]) != HIGH) {
        delay(1);
    }
    stopMotor();

    currentPosition = nextPosition;

    delay(2000);
}

void blink(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);
        delay(250);
    }
}

void goForward() {
    analogWrite(MOTOR_SPEED_PIN, MOTOR_SPEED);
    digitalWrite(MOTOR_DIRECTION_PIN_1, LOW);
    digitalWrite(MOTOR_DIRECTION_PIN_2, HIGH);
}

void goBackward() {
    analogWrite(MOTOR_SPEED_PIN, MOTOR_SPEED);
    digitalWrite(MOTOR_DIRECTION_PIN_1, HIGH);
    digitalWrite(MOTOR_DIRECTION_PIN_2, LOW);
}

void stopMotor() {
    analogWrite(MOTOR_SPEED_PIN, 0xFF);
    digitalWrite(MOTOR_DIRECTION_PIN_1, HIGH);
    digitalWrite(MOTOR_DIRECTION_PIN_2, HIGH);
}
