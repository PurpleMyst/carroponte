// Servono tre pin per il driver:
// - Uno per la velocità del motore (PWM) nonché per l'abilitazione;
// - Due per la direzione del motore.
#define MOTOR_SPEED_PIN 9
#define MOTOR_DIRECTION_PIN_1 10
#define MOTOR_DIRECTION_PIN_2 11

// Ciascun microswitch è collegato ad una determinata pin; quando una di queste è HIGH, il motore si
// trova in corrispondenza di quel finecorsa. Si assume che l'array sia ordinato da sinistra verso
// destra.
int positionPins[] = {2, 3, 4, 5, 6};

void setup() {
    // Apriamo la comunicazione seriale.
    Serial.begin(9600);

    // Lasciando la pin analogica 0 sconnessa, leggendo da essa otterremo valori praticamente
    // aleatori. Perciò, usiamo tale valore come seed.
    randomSeed(analogRead(0));

    // Inizializziamo i pin.
    pinMode(MOTOR_SPEED_PIN, OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN_1, OUTPUT);
    pinMode(MOTOR_DIRECTION_PIN_2, OUTPUT);
    int positions = sizeof(positionPins) / sizeof(positionPins[0]);
    for (int i = 0; i < positions; i++) {
        pinMode(positionPins[i], INPUT);
    }
}

void loop() {
    // Calcoliamo il numero di posizioni.
    int positions = sizeof(positionPins) / sizeof(positionPins[0]);

    // Troviamo la posizione corrente.
    int currentPosition = -1;
    for (int i = 0; i < 5; i++) {
        // Se il microswitch è HIGH, il motore si trova in corrispondenza di quel finecorsa.
        if (digitalRead(positionPins[i]) == HIGH) {
            printTime();
            Serial.print("Motor is at position ");
            Serial.println(i);
            if (currentPosition != -1) {
                printTime();
                Serial.println("The motor is somehow in two positions at once, WTF?");
                return;
            }
            currentPosition = i;
        }
    }
    if (currentPosition == -1) {
        printTime();
        Serial.println("Motor is not at any position, WTF?");
        return;
    }

    // Spostiamoci ad un altra posizione randomica.
    int nextPosition = random(0, positions);
    printTime();
    Serial.print("Moving to position ");
    Serial.println(nextPosition);

    if (nextPosition == currentPosition) {
        return;
    } else if (nextPosition > currentPosition) {
        turnClockwise();
    } else {
        turnCounterClockwise();
    }

    // Spin mentre aspettiamo che colpiamo il finecorsa.
    while (digitalRead(positionPins[nextPosition]) != HIGH) {
        delay(1);
    }
    printTime();
    Serial.println("Reached destination");

    // Fermiamo il motore e riposiamo per qualche secondo.
    stopMotor();

    delay(2000);
}

void printTime() {
    Serial.print("[")
    Serial.print(millis());
    Serial.print("] ");
}

void turnClockwise() {
    printTime();
    Serial.println("Turning clockwise");
    analogWrite(MOTOR_SPEED_PIN, 255);
    digitalWrite(MOTOR_DIRECTION_PIN_1, LOW);
    digitalWrite(MOTOR_DIRECTION_PIN_2, HIGH);
}

void turnCounterClockwise() {
    printTime();
    Serial.println("Turning counter-clockwise");
    analogWrite(MOTOR_SPEED_PIN, 255);
    digitalWrite(MOTOR_DIRECTION_PIN_1, HIGH);
    digitalWrite(MOTOR_DIRECTION_PIN_2, LOW);
}

void stopMotor() {
    printTime();
    Serial.println("Stopping motor");
    analogWrite(MOTOR_SPEED_PIN, 0);
    digitalWrite(MOTOR_DIRECTION_PIN_1, HIGH);
    digitalWrite(MOTOR_DIRECTION_PIN_2, HIGH);
}
