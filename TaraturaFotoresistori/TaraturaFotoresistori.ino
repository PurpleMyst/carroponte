uint8_t pins[4] = {A12, A13, A14, A15};
double lightMeans[4] = {0, 0, 0, 0};
double darkMeans[4] = {0, 0, 0, 0};

double fraction = 1;

const uint64_t TOTAL_INTERVAL = 5000;

void setup() {
    Serial.begin(9600);
    while (!Serial) {
    };
    Serial.println("");
    for (int i = 0; i < 4; ++i) {
        uint8_t pin = pins[i];
        double sum = 0;
        uint64_t count = 0;
        for (uint64_t start = millis(); millis() < start + TOTAL_INTERVAL; ) {
            uint64_t value = analogRead(pin);
            sum += value;
            count++;
        }
        double mean = sum / count;
        lightMeans[i] = mean;

        // Calculate the variance.
        double sum2 = 0;
        count = 0;
        for (uint64_t start = millis(); millis() < start + TOTAL_INTERVAL;) {
            uint64_t value = analogRead(pin);
            sum2 += (value - mean) * (value - mean);
            count++;
        }
        double variance =sum2 / count;
        double stdev = sqrt(variance);
        Serial.print(pins[i]);
        Serial.print('\t');
        Serial.print(mean);
        Serial.print('\t');
        Serial.print(variance);
        Serial.print('\t');
        Serial.println(stdev);
    }
}

void loop() {
    // for (int i = 0; i < 4 ; ++ i) {
    //   if (i != 0) Serial.print(' ');
    //   Serial.print(analogRead(pins[i]) < fraction * means[i]);
    // }
    // Serial.println("");
    // if (Serial.available()) {
    //   String line = Serial.readStringUntil('\n');
    //   line.trim();
    //   fraction = line.toInt() / 100.0;
    // }
}
