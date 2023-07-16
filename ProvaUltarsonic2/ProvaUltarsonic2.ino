#define MOTOR_SPEED_PIN 9
#define MOTOR_DIRECTION_PIN_1 10
#define MOTOR_DIRECTION_PIN_2 11
#define MOTOR_SPEED 150
#define TRIG_PIN 7
#define ECHO_PIN 8
#define POSITIONS 3

double desiredPosition = 0.0;
double positions[POSITIONS] = { 10, 15.4, 18.5 };
double speed = MOTOR_SPEED;

double measure() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  double d = ((double)pulseIn(ECHO_PIN, HIGH)) / 58.3;
  return d;
}

void goHome();

void setup() {
  Serial.begin(9600);
  while (!Serial) {};
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOTOR_SPEED_PIN, OUTPUT);
  pinMode(MOTOR_DIRECTION_PIN_1, OUTPUT);
  pinMode(MOTOR_DIRECTION_PIN_2, OUTPUT);

  blink(7);

  // go home
  goHome();

  // test each position
  for (int i = 0; i < POSITIONS; i++) {
    Serial.print("Calibration for position:");
    Serial.println(i);
    blink(i + 1);
    desiredPosition = positions[i];
    move();
    delay(2500);
  }

  // go home again and begin loop() with random positions
  goHome();

  // set random seed
  randomSeed(analogRead(0));
}

void loop() {
  int idx = random(0, POSITIONS);
  blink(idx + 1);
  desiredPosition = positions[idx];
  move();
  delay(3000);
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
  analogWrite(MOTOR_SPEED_PIN, constrain(abs(speed), 0, 0xFF));
  digitalWrite(MOTOR_DIRECTION_PIN_1, LOW);
  digitalWrite(MOTOR_DIRECTION_PIN_2, HIGH);
}

void goBackward() {
  analogWrite(MOTOR_SPEED_PIN, constrain(abs(speed), 0, 0xFF));
  digitalWrite(MOTOR_DIRECTION_PIN_1, HIGH);
  digitalWrite(MOTOR_DIRECTION_PIN_2, LOW);
}

void stopMotor() {
  analogWrite(MOTOR_SPEED_PIN, 0xFF);
  digitalWrite(MOTOR_DIRECTION_PIN_1, LOW);
  digitalWrite(MOTOR_DIRECTION_PIN_2, LOW);
}

void move() {
  speed = MOTOR_SPEED;

  unsigned long lastUpdateTime = millis();
  double lastUpdatePos = measure();
  for (;;) {
    double pos = measure();
    Serial.print(pos);
    Serial.print(" => ");
    Serial.println(desiredPosition);
    if (abs(desiredPosition - pos) <= 0.15) {
      stopMotor();
      break;
    }
    if (millis() >= lastUpdateTime + 2000) {
      lastUpdateTime = millis();
      if (abs(lastUpdatePos - pos) <= 0.5) {
        Serial.println("BOOST!!!");
        analogWrite(MOTOR_SPEED_PIN, 0xFF);
        delay(100);
        analogWrite(MOTOR_SPEED_PIN, MOTOR_SPEED);
      } else {
        speed = MOTOR_SPEED;
      }
      
      lastUpdatePos = pos;
    }
    if (desiredPosition > pos) goForward();
    else goBackward();
  }
}

void goHome() {
  desiredPosition = 7.0;
  move();
}
