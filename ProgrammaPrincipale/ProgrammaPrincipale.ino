#define nullptr NULL
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>

#include <stddef.h>
#include <Ethernet.h>
#include <Modbus.h>
#include <ModbusEthernet.h>
#include <Servo.h>
#include <Ultrasonic.h>

// Pin di controllo del motore DC per l'asse X.
#define X_SPEED_PIN 9
#define X_D1_PIN    10
#define X_D2_PIN    11

// Distanze per le varie posizioni dell'asse X.
#define X_POSITIONS 3
int xPositionDistances[X_POSITIONS] = {2, 3, 4};

#define TRIGGER_PIN 12
#define ECHO_PIN    13
#define X_TOLERANCE 1
Ultrasonic xDistanceSensor(TRIGGER_PIN, ECHO_PIN);

// Velocità motore dell'asse X.
#define X_SPEED 180

// Pin di controllo del servomotore per l'asse Y.
#define Y_SERVO_PIN 0

// Pin di controllo del servomotore per l'asse Z.
#define Z_SERVO_PIN 0

// Pin di controllo per l'abilitazione dell'elettromagnete.
#define ELECTROMAGNET_PIN 13

// Soglia scelta per la rilevazione dei foto-resistori.
#define PHOTO_THRESHOLD 100

// Quanti millisecondi aspettiamo per il movimento dei servo?
#define SERVO_MOVEMENT_TIME 5000

struct Photoresistor {
    // Pin di lettura del sensore.
    int pin;

    // Posizione del sensore rispetto al piano X-Y.
    int x;
    int y;
};

#define BOAT_PHOTORESISTORS               2
#define PORT_PHOTORESISTORS_FOR_UNLOADING 2
#define PORT_PHOTORESISTORS_FOR_LOADING   2

#define Y_ANGLE_FOR_FIRST_ZONE  0
#define Y_ANGLE_FOR_SECOND_ZONE 90

#define Z_ANGLE_FOR_RETRACTION 0
#define Z_ANGLE_FOR_EXTENSION 180

// Array di foto-resistori per:
// 1) Le zone presenti sulla nave;
// 2) Le zone presenti sul porto in cui vengono posti i prossimi container da caricare.
// 3) Le zone presenti sul porto in cui vengono posti i container scaricati.
Photoresistor boatPhotoresistors[BOAT_PHOTORESISTORS] = {
    {A0, 0, Y_ANGLE_FOR_FIRST_ZONE},
    {A1, 0, Y_ANGLE_FOR_SECOND_ZONE},
};
Photoresistor portPhotoresistorsForUnloading[PORT_PHOTORESISTORS_FOR_UNLOADING] = {
    {A2, 1, Y_ANGLE_FOR_FIRST_ZONE},
    {A3, 1, Y_ANGLE_FOR_SECOND_ZONE},
};
Photoresistor portPhotoresistorsForLoading[PORT_PHOTORESISTORS_FOR_LOADING] = {
    {A4, 2, Y_ANGLE_FOR_FIRST_ZONE},
    {A5, 2, Y_ANGLE_FOR_SECOND_ZONE},
};

enum Stato {
    // Il controllo manuale è abilitato mediante SCADA.
    MC,

    // Stato di controllo automatico con nessun container rilevato.
    AC_IDLE,

    // Stato di controllo automatico con necessita di movimento sul piano X-Y.
    AC_GO_XY_PICKUP,

    // Stato di controllo automatico con necessita di movimento sul piano Z.
    AC_GO_Z_PICKUP,

    // Stato di controllo automatico con avvenuto aggancio e necessita di retrazione.
    AC_PICKEDUP,

    // Stato di controllo automatico con necessita di movimento sul piano X-Y per drop off.
    AC_GO_XY_DROP,

    // Stato di controllo automatico con necessita di movimento sul piano Z per drop off.
    AC_GO_Z_DROP,

    // Stato di controllo automatico in cui il container è stato scaricato e dobbiamo tornare a
    // casa.
    AC_GO_HOME_KID,
} state;

// Definiamo i parametri di rete; l'indirizzo MAC è stato letto un adesivo sul PLC, mentre
// l'indirizzo IP va assegnato nel router.
byte mac[] = {0xE0, 0xDC, 0xA0, 0x14, 0x07, 0x80};
byte ip[]  = {192, 168, 1, 177};

ModbusEthernet mb;

// La posizione in X è in un range da 0 ad 1.
int xPosition = 0;

// La posizione in Y è rappresentata come un angolo in gradi da 0 a 180.
int yPosition = 0;
Servo servoY;

// La posizione in Z è rappresentata come un angolo in gradi da 0 a 180.
int zPosition = 0;
Servo servoZ;

// Dove dobbiamo lasciare il container che abbiamo?
int posizioneDropoffX;
int posizioneDropoffY;

// Lo stato dell'elettromagnete.
bool electromagnetEnabled = false;

unsigned long deadlineForServoMovement = 0;

void setup() {
    Serial.begin(9600);

    servoY.attach(Y_SERVO_PIN);
    servoZ.attach(Z_SERVO_PIN);

    // Configurazione del server Ethernet e del server Modbus TCP per agire da slave.
    mb.config(mac, ip);

    // Configuriamo gli indirizzi Modbus.
    int photoresistors = BOAT_PHOTORESISTORS + PORT_PHOTORESISTORS_FOR_UNLOADING + PORT_PHOTORESISTORS_FOR_LOADING;
    mb.addCoil(0, false); // controllo manuale
    mb.addCoil(1, false); // stato elettromagnete desiderato
    mb.addIsts(0, false); // stato elettromagnete corrente
    mb.addIreg(0, 0); // posizione corrente x
    mb.addIreg(1, 0); // posizione corrente y
    mb.addIreg(2, 0); // posizione corrente z
    for (int i = 0; i < photoresistors; i++) {
        mb.addIreg(3 + i, 0); // lettura foto-resistori
    }
    mb.addHreg(0, 0); // posizione desiderata x
    mb.addHreg(1, 0); // posizione desiderata y
    mb.addHreg(2, 0); // posizione desiderata z

    // Inizializzazione dei pin.
    pinMode(X_SPEED_PIN, OUTPUT);
    pinMode(X_D1_PIN, OUTPUT);
    pinMode(X_D2_PIN, OUTPUT);
    pinMode(Y_SERVO_PIN, OUTPUT);
    pinMode(Z_SERVO_PIN, OUTPUT);
    pinMode(ELECTROMAGNET_PIN, OUTPUT);
    for (int i = 0; i < BOAT_PHOTORESISTORS; i++) {
        pinMode(boatPhotoresistors[i].pin, INPUT);
    }
    for (int i = 0; i < PORT_PHOTORESISTORS_FOR_UNLOADING; i++) {
        pinMode(portPhotoresistorsForUnloading[i].pin, INPUT);
    }
    for (int i = 0; i < PORT_PHOTORESISTORS_FOR_LOADING; i++) {
        pinMode(portPhotoresistorsForLoading[i].pin, INPUT);
    }
    for (int i = 0; i < X_POSITIONS; i++) {
        pinMode(xPositions[i].pin, INPUT_PULLUP);
    }

    goHome();
}

void goHome() {
    goBackwardsInX();
    moveY(0);
    moveZ(Z_ANGLE_FOR_RETRACTION);
    setElectromagnet(false);
    state = AC_GO_HOME_KID;
}

void loop() {
    // Poll del Modbus + aggiornamento dei dati che spettano a noi.
    mb.task();
    mb.setIsts(0, electromagnetEnabled);
    int irAddress = 0;
    mb.setIreg(irAddress++, xPosition);
    mb.setIreg(irAddress++, yPosition);
    mb.setIreg(irAddress++, zPosition);
    for (int i = 0; i < BOAT_PHOTORESISTORS; i++) {
        mb.setIreg(irAddress++, analogRead(boatPhotoresistors[i].pin));
    }
    for (int i = 0; i < PORT_PHOTORESISTORS_FOR_UNLOADING; i++) {
        mb.setIreg(irAddress++, analogRead(portPhotoresistorsForUnloading[i].pin));
    }
    for (int i = 0; i < PORT_PHOTORESISTORS_FOR_LOADING; i++) {
        mb.setIreg(irAddress++, analogRead(portPhotoresistorsForLoading[i].pin));
    }

    // Se siamo fermi/arrivati, possiamo controllare se passare al controllo manuale.
    if (arrived()) {
        if (mb.coil(0)) {
            state = MC;
        }
    }

    switch (state) {
        case MC:
            if (arrived()) {
                if (!mb.coil(0)) {
                    goHome();
                    break;
                }

                moveXYZ(mb.hreg(0), mb.hreg(1), mb.hreg(2));
                setElectromagnet(mb.coil(1));
            }

            break;

        case AC_IDLE:
            // Controlliamo se ci sono carichi sulla nave, se si andiamoli a prendere.
            for (int i = 0; i < BOAT_PHOTORESISTORS; i++) {
                if (containerDetected(boatPhotoresistors[i])) {
                    // Troviamo una posizione di drop off.
                    posizioneDropoffX = posizioneDropoffY = -1;
                    for (int j = 0; j < PORT_PHOTORESISTORS_FOR_UNLOADING; j++) {
                        if (!containerDetected(portPhotoresistorsForUnloading[j])) {
                            posizioneDropoffX = portPhotoresistorsForUnloading[j].x;
                            posizioneDropoffY = portPhotoresistorsForUnloading[j].y;
                            break;
                        }
                    }
                    if (posizioneDropoffX == -1) {
                        // Non abbiamo trovato una posizione di drop off, quindi non possiamo fare nulla.
                        break;
                    }

                    moveXY(boatPhotoresistors[i].x, boatPhotoresistors[i].y);
                    state = AC_GO_XY_PICKUP;
                    break;
                }
            }

            // Se non ci sono carichi sulla nave, allora possiamo caricare ciò che sta sul porto.
            for (int i = 0; i < PORT_PHOTORESISTORS_FOR_LOADING; i++) {
                if (containerDetected(portPhotoresistorsForLoading[i])) {
                    // Troviamo una posizione di drop off.
                    posizioneDropoffX = posizioneDropoffY = -1;
                    for (int j = 0; j < BOAT_PHOTORESISTORS; j++) {
                        if (!containerDetected(boatPhotoresistors[j])) {
                            posizioneDropoffX = boatPhotoresistors[j].x;
                            posizioneDropoffY = boatPhotoresistors[j].y;
                            break;
                        }
                    }
                    if (posizioneDropoffX == -1) {
                        // Non abbiamo trovato una posizione di drop off, quindi non possiamo fare nulla.
                        break;
                    }

                    moveXY(portPhotoresistorsForLoading[i].x, portPhotoresistorsForLoading[i].y);
                    state = AC_GO_XY_PICKUP;
                    break;
                }
            }


            break;

        case AC_GO_XY_PICKUP:
            if (arrived()) {
                moveZ(Z_ANGLE_FOR_EXTENSION);
                setElectromagnet(true);
                state = AC_GO_Z_PICKUP;
            }

            break;

        case AC_GO_Z_PICKUP:
            if (arrived()) {
                moveZ(Z_ANGLE_FOR_RETRACTION);
                state = AC_PICKEDUP;
            }

            break;

        case AC_PICKEDUP:
            if (arrived()) {
                moveXY(posizioneDropoffX, posizioneDropoffY);
                state = AC_GO_XY_DROP;
            }

            break;

        case AC_GO_XY_DROP:
            if (arrived()) {
                moveZ(Z_ANGLE_FOR_EXTENSION);
                state = AC_GO_Z_DROP;
            }

            break;

        case AC_GO_Z_DROP:
            if (arrived()) {
                goHome();
            }

            break;

        case AC_GO_HOME_KID:
            if (arrived()) {
                state = AC_IDLE;
            }

            break;
    }
}

void goBackwardsInX() {
    digitalWrite(X_D1_PIN, HIGH);
    digitalWrite(X_D2_PIN, LOW);
    analogWrite(X_SPEED_PIN, X_SPEED);
}

void goForwardInX() {
    digitalWrite(X_D1_PIN, LOW);
    digitalWrite(X_D2_PIN, HIGH);
    analogWrite(X_SPEED_PIN, X_SPEED);
}

void stopX() {
    analogWrite(X_SPEED_PIN, 0xFF);
    digitalWrite(X_D1_PIN, LOW);
    digitalWrite(X_D2_PIN, LOW);
}

void moveX(int targetX) {
    if (xPosition < targetX) {
        goForwardInX();
    } else {
        goBackwardsInX();
    }
    xPosition = targetX;
}

void moveY(int targetY) {
    targetY = constrain(targetY, 0, 180);
    servoY.write(targetY);
    deadlineForServoMovement = millis() + SERVO_MOVEMENT_TIME;
    yPosition = targetY;
}

void moveZ(int targetZ) {
    targetZ = constrain(targetZ, 0, 180);
    servoZ.write(targetZ);
    deadlineForServoMovement = millis() + SERVO_MOVEMENT_TIME;
    zPosition = targetZ;
}

void moveXY(int targetX, int targetY) {
    moveY(targetY);
    moveX(targetX);
}

void moveXYZ(int targetX, int targetY, int targetZ) {
    moveZ(targetZ);
    moveXY(targetX, targetY);
}

bool arrived() {
    int xDistance = xDistanceSensor.read();
    if (abs(xDistance - xPosition) > X_TOLERANCE) return false;
    stopX();
    return (millis() >= deadlineForServoMovement);
}

void setElectromagnet(bool enabled) {
    electromagnetEnabled = enabled;
    digitalWrite(ELECTROMAGNET_PIN, enabled ? HIGH : LOW);
}

bool containerDetected(Photoresistor photoresistor) {
    return analogRead(photoresistor.pin) > PHOTO_THRESHOLD;
}
