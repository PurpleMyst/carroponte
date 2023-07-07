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

// Pin di controllo del motore DC per l'asse X.
#define X_SPEED_PIN 0
#define X_D1_PIN    0
#define X_D2_PIN    0

// Velocità asse X
#define X_SPEED 255

// Pin di finecorsa inizio e fine asse X.
#define X_START_PIN 0
#define X_END_PIN   0

// Pin di controllo del servomotore per l'asse Y.
#define Y_SERVO_PIN 0

// Pin di controllo del servomotore per l'asse Z.
#define Z_SERVO_PIN 0

// Pin di controllo per l'abilitazione dell'elettromagnete.
#define ELECTROMAGNET_PIN 0

// Soglia scelta per la rilevazione dei foto-resistori.
#define PHOTO_THRESHOLD 0

// Quanto tempo aspettiamo per il movimento dei servo?
#define SERVO_MOVEMENT_TIME 1000

struct Photoresistor {
    // Pin di lettura del sensore.
    int pin;

    // Posizione del sensore rispetto al piano X-Y.
    int x;
    int y;
};

// Array di foto-resistori per:
// 1) Le zone presenti sulla nave;
// 2) Le zone presenti sul porto in cui vengono posti i prossimi container da caricare.
// 3) Le zone presenti sul porto in cui vengono posti i container scaricati.
Photoresistor boatPhotoresistors[0];
Photoresistor portPhotoresistorsForUnloading[0];
Photoresistor portPhotoresistorsForLoading[0];

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
byte ip[] = {192, 168, 1, 177};

ModbusEthernet mb;

// La posizione in X è in un range da 0 ad 1.
double posizioneX = 0;

// La posizione in Y è rappresentata come un angolo in gradi da 0 a 180.
byte posizioneY = 0;
Servo servoY;

// La posizione in Z è rappresentata come un angolo in gradi da 0 a 180.
byte posizioneZ = 0;
Servo servoZ;

// Dove dobbiamo lasciare il container che abbiamo?
double posizioneDropoffX;
byte posizioneDropoffY;

// Lo stato dell'elettromagnete.
bool electromagnetEnabled = false;

// Il tempo necessario per andare dall'inizio dell'asse x alla fine durante la calibrazione.
unsigned long timeForWholeX = 0;

// Quando finirà il movimento corrente?
unsigned long deadlineForDCMovement = 0;
unsigned long deadlineForServoMovement = 0;

void setup() {
    Serial.begin(9600);

    servoY.attach(Y_SERVO_PIN);
    servoZ.attach(Z_SERVO_PIN);

    // Configurazione del server Ethernet e del server Modbus TCP per agire da slave.
    mb.config(mac, ip);

    // Configuriamo gli indirizzi Modbus.
    int photoresistors = (sizeof(boatPhotoresistors) + sizeof(portPhotoresistorsForUnloading) + sizeof(portPhotoresistorsForLoading)) / sizeof(Photoresistor);
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
    for (int i = 0; i < sizeof(boatPhotoresistors); i++) {
        pinMode(boatPhotoresistors[i].pin, INPUT);
    }
    for (int i = 0; i < sizeof(portPhotoresistorsForUnloading); i++) {
        pinMode(portPhotoresistorsForUnloading[i].pin, INPUT);
    }
    for (int i = 0; i < sizeof(portPhotoresistorsForLoading); i++) {
        pinMode(portPhotoresistorsForLoading[i].pin, INPUT);
    }

    // Calibrazione movimento asse X.
    calibrateXAxis();
}

void loop() {
    // Poll del Modbus + aggiornamento dei dati che spettano a noi.
    mb.task();
    mb.setIsts(0, electromagnetEnabled);
    int irAddress = 0;
    mb.setIreg(irAddress++, 0xFFFF * posizioneX);
    mb.setIreg(irAddress++, posizioneY);
    mb.setIreg(irAddress++, posizioneZ);
    for (int i = 0; i < sizeof(boatPhotoresistors) / sizeof(Photoresistor); i++) {
        mb.setIreg(irAddress++, analogRead(boatPhotoresistors[i].pin));
    }
    for (int i = 0; i < sizeof(portPhotoresistorsForUnloading) / sizeof(Photoresistor); i++) {
        mb.setIreg(irAddress++, analogRead(portPhotoresistorsForUnloading[i].pin));
    }
    for (int i = 0; i < sizeof(portPhotoresistorsForLoading) / sizeof(Photoresistor); i++) {
        mb.setIreg(irAddress++, analogRead(portPhotoresistorsForLoading[i].pin));
    }

    // Se siamo fermi/arrivati, possiamo controllare se passare al controllo manuale.
    if (checkDeadlines()) {
        if (mb.coil(0)) {
            state = MC;
        }
    }

    switch (state) {
        case MC:
            if (checkDeadlines()) {
                if (!mb.coil(0)) {
                    setElectromagnet(false);
                    moveXYZ(0, 0, 0);
                    state = AC_GO_HOME_KID;
                    break;
                }

                moveXYZ(((double) mb.hreg(0)) / ((double) 0xFFFF), mb.hreg(1), mb.hreg(2));
                setElectromagnet(mb.coil(1));
            }

            break;

        case AC_IDLE:
            // Controlliamo se ci sono carichi sulla nave, se si andiamoli a prendere.
            for (int i = 0; i < sizeof(boatPhotoresistors) / sizeof(Photoresistor); i++) {
                if (containerDetected(boatPhotoresistors[i])) {
                    // Troviamo una posizione di drop off.
                    posizioneDropoffX = posizioneDropoffY = -1;
                    for (int j = 0; j < sizeof(portPhotoresistorsForUnloading) / sizeof(Photoresistor); j++) {
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
            for (int i = 0; i < sizeof(portPhotoresistorsForLoading) / sizeof(Photoresistor); i++) {
                if (containerDetected(portPhotoresistorsForLoading[i])) {
                    // Troviamo una posizione di drop off.
                    posizioneDropoffX = posizioneDropoffY = -1;
                    for (int j = 0; j < sizeof(boatPhotoresistors) / sizeof(Photoresistor); j++) {
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
            if (checkDeadlines()) {
                moveZ(180);
                setElectromagnet(true);
                state = AC_GO_Z_PICKUP;
            }

            break;

        case AC_GO_Z_PICKUP:
            if (checkDeadlines()) {
                moveZ(0);
                state = AC_PICKEDUP;
            }

            break;

        case AC_PICKEDUP:
            if (checkDeadlines()) {
                moveXY(posizioneDropoffX, posizioneDropoffY);
                state = AC_GO_XY_DROP;
            }

            break;

        case AC_GO_XY_DROP:
            if (checkDeadlines()) {
                moveZ(180);
                state = AC_GO_Z_DROP;
            }

            break;

        case AC_GO_Z_DROP:
            if (checkDeadlines()) {
                setElectromagnet(false);
                moveXYZ(0, 0, 0);
                state = AC_GO_HOME_KID;
            }

            break;

        case AC_GO_HOME_KID:
            if (checkDeadlines()) {
                state = AC_IDLE;
            }

            break;
    }
}

void calibrateXAxis() {
    goBackwardsInX();
    while (digitalRead(X_START_PIN) != HIGH) {
        delay(10);
    }
    stopX();
    unsigned long startTime = millis();
    goForwardInX();
    while (digitalRead(X_END_PIN) != HIGH) {
        delay(10);
    }
    stopX();
    unsigned long endTime = millis();
    timeForWholeX = endTime - startTime;
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
    digitalWrite(X_D1_PIN, LOW);
    digitalWrite(X_D2_PIN, LOW);
    analogWrite(X_SPEED_PIN, 0);
}

void moveXY(double targetX, byte targetY) {
    // Non si sa mai.
    targetX = constrain(targetX, 0, 1);
    targetY = constrain(targetY, 0, 180);

    unsigned long timeToWait = ((double) timeForWholeX) * abs(posizioneX - targetX);
    if (posizioneX < targetX) {
        goForwardInX();
    } else {
        goBackwardsInX();
    }
    servoY.write(targetY);
    deadlineForServoMovement = millis() + SERVO_MOVEMENT_TIME;
    deadlineForDCMovement = millis() + timeToWait;
    posizioneX = targetX;
    posizioneY = targetY;
}

void moveZ(byte targetZ) {
    targetZ = constrain(targetZ, 0, 180);
    servoZ.write(targetZ);
    deadlineForServoMovement = millis() + SERVO_MOVEMENT_TIME;
    posizioneZ = targetZ;
}

void moveXYZ(double targetX, byte targetY, byte targetZ) {
    moveZ(targetZ);
    moveXY(targetX, targetY);
}

bool checkDeadlines() {
    if (millis() > deadlineForDCMovement) {
        stopX();
    } else {
        return false;
    }
    return millis() > deadlineForServoMovement;
}

void setElectromagnet(bool enabled) {
    electromagnetEnabled = enabled;
    digitalWrite(ELECTROMAGNET_PIN, enabled ? HIGH : LOW);
}

bool containerDetected(Photoresistor photoresistor) {
    return analogRead(photoresistor.pin) > PHOTO_THRESHOLD;
}
