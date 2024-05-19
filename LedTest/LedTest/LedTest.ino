#include <Arduino_LED_Matrix.h>
#include "LedManager.h"

LedManager ledManager;

void setup() {
    Serial.begin(115200);
    while (!Serial);
}

void loop() {
    ledManager.ProcessLedMatrix(nullptr, millis());
}
