// Servono tre pin per il driver:
// - Uno per la velocità del motore (PWM) nonché per l'abilitazione;
// - Due per la direzione del motore.
#define MOTOR_SPEED_PIN 9
#define MOTOR_DIRECTION_PIN_1 10
#define MOTOR_DIRECTION_PIN_2 11

// La velocità del motore è controllata tramite PWM, quindi può assumere valori da 0 a 255.
#define MOTOR_SPEED 255

// Questa volta abbiamo solo un finecorsa per la fine.
#define POSITION_PIN 3

// Variabile statica che immagazina la posizione attuale del motore.
double currentPosition;

// Quanto tempo serve per andare dal primo finecorsa all'ultimo?
int timeForWhole;

void setup() {
    // Apriamo la comunicazione seriale.
    Serial.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);

    // Lasciando la pin analogica 0 sconnessa, leggendo da essa otterremo valori praticamente
    // aleatori. Perciò, usiamo tale valore come seed.
    randomSeed(analogRead(0));

    // Inizializziamo i pin.
    pinMode(MOTOR_SPEED_PIN, OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN_1, OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN_2, OUTPUT);
    pinMode(POSITION_PIN, INPUT);

    // Supponiamo di essere nella posizione iniziale.
    // Spostiamoci alla posizione finale.
    printTime();
    Serial.println("Measuring time for a whole");
    int start = millis();
    while (digitalRead(POSITION_PIN) != HIGH) {
        turnClockwise();
    }
    stopMotor();
    int end = millis();

    // Misurando quanto tempo abbiamo impiegato a fare questo spostamento, possiamo inferire il
    // tempo per spostamenti parziali.
    timeForWhole = end - start;
    currentPosition = 1.0;
}

void loop() {
    // Spostiamoci ad un altra posizione randomica.
    double nextPosition = ((double) random(0, 1024)) / 1023.f;
    double diff = abs(nextPosition - currentPosition);
    int timeToWait = diff * timeForWhole;

    printTime();
    Serial.print("Moving to position ");
    Serial.print(nextPosition);
    Serial.print(" (diff of ");
    Serial.print(diff);
    Serial.print(") by turning for ");
    Serial.print(timeToWait);
    Serial.println("ms");

    if (nextPosition == currentPosition) {
        return;
    } else if (nextPosition > currentPosition) {
        turnClockwise();
    } else {
        turnCounterClockwise();
    }
    delay(timeToWait);
    stopMotor();
    currentPosition = nextPosition;
    printTime();
    Serial.println("Reached position, waiting for 2s");

    delay(2000);
}

void printTime() {
    Serial.print("[");
    Serial.print(millis());
    Serial.print("] ");
}

void turnClockwise() {
    printTime();
    Serial.println("Turning clockwise");
    analogWrite(MOTOR_SPEED_PIN, MOTOR_SPEED);
    digitalWrite(MOTOR_DIRECTION_PIN_1, LOW);
    digitalWrite(MOTOR_DIRECTION_PIN_2, HIGH);
}

void turnCounterClockwise() {
    printTime();
    Serial.println("Turning counter-clockwise");
    analogWrite(MOTOR_SPEED_PIN, MOTOR_SPEED);
    digitalWrite(MOTOR_DIRECTION_PIN_1, HIGH);
    digitalWrite(MOTOR_DIRECTION_PIN_2, LOW);
}

void stopMotor() {
    printTime();
    Serial.println("Stopping motor");
    // Mettiamo la SPEED al massimo per fare uso del fast stop.
    analogWrite(MOTOR_SPEED_PIN, 255);
    digitalWrite(MOTOR_DIRECTION_PIN_1, HIGH);
    digitalWrite(MOTOR_DIRECTION_PIN_2, HIGH);
}