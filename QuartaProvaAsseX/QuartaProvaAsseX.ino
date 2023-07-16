#include <Ultrasonic.h>
#include <PID_v1.h>

#define MOTOR_SPEED_PIN 9
#define MOTOR_DIRECTION_PIN_1 10
#define MOTOR_DIRECTION_PIN_2 11
// #define MOTOR_SPEED 120
#define X_TOLERANCE 0.3
#define POSITIONS 3

#define TRIG_PIN 7
#define ECHO_PIN 8

double pid_setpoint, pid_input, pid_output;
PID pid(&pid_input, &pid_output, &pid_setpoint, 20, 50, 10, DIRECT);

double measure() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  double d = ((double)pulseIn(ECHO_PIN, HIGH)) / 58.3;
  return d;
}

double positions[POSITIONS] = { 10, 16, 20 };

void goHome();

void setup() {
  // initialize serial communication
  Serial.begin(9600);
  while (!Serial) {};
  pinMode(TRIG_PIN, OUTPUT);
  pid.SetMode(AUTOMATIC);
  pid.SetOutputLimits(-180, 180);

  // configure pins
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
    pid_setpoint = positions[i];
    move();
    delay(2500);
  }

  // go home again and begin loop() with random positions
  goHome();

  // set random seed
  randomSeed(analogRead(0));
}

void loop() {
  pid_setpoint = positions[random(0, POSITIONS)];
  move();
  delay(5000);
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
  analogWrite(MOTOR_SPEED_PIN, constrain(abs(pid_output), 0, 0xFF));
  digitalWrite(MOTOR_DIRECTION_PIN_1, LOW);
  digitalWrite(MOTOR_DIRECTION_PIN_2, HIGH);
}

void goBackward() {
  analogWrite(MOTOR_SPEED_PIN, constrain(abs(pid_output), 0, 0xFF));
  digitalWrite(MOTOR_DIRECTION_PIN_1, HIGH);
  digitalWrite(MOTOR_DIRECTION_PIN_2, LOW);
}

void stopMotor() {
  analogWrite(MOTOR_SPEED_PIN, 0xFF);
  digitalWrite(MOTOR_DIRECTION_PIN_1, HIGH);
  digitalWrite(MOTOR_DIRECTION_PIN_2, HIGH);
}

void move() {
  for (;;) {
    pid_input = measure();
    if (abs(pid_input - pid_setpoint) <= 0.33) {
      stopMotor();
      break;
    }
    pid.Compute();
    Serial.print(pid_setpoint);
    Serial.print(' ');
    Serial.print(pid_input);
    Serial.print(' ');
    Serial.println(pid_output);
    if (pid_output > 0) goForward();
    else if (pid_output < 0) goBackward();
    else stopMotor();
  }
}

void goHome() {
  pid_setpoint = 7.0;
  move();
}
