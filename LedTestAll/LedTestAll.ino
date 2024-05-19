#include <Arduino.h>
#include <WiFiS3.h>
#include "WebClientManager.h"
#include "LedManager.h"

#define WATER_PUMP

#include "Common.h"

LedManager ledManager;

uint64_t nextUpdate = 0;
float temp = -5;
float humid = -5;
int stage = 0;

void setup()
{
    Serial.begin(115200);
    while (!Serial);
  // put your setup code here, to run once:
    ledManager.Initialize();
    ledManager.StartupSequence();
    //ledManager.SetIsRaining(false);
    ledManager.SetPumpOne(true);
    ledManager.SetPumpTwo(true);
}

void loop() 
{
    uint64_t currMillis = millis();
    DoUpdate(currMillis);
    ledManager.ProcessLedMatrix(nullptr, currMillis);
}

void DoUpdate(uint64_t currMillis)
{
    if (currMillis < nextUpdate)
        return;

    temp++;
    humid++;


    //ledManager.SetTemperature(temp);
    //ledManager.SetHumidity(humid);

    if (temp > 100)
    {
        temp = 0;
        humid = 0;
        ledManager.ShutdownSequence();
    }

    //Serial.print("Temp: ");
    //Serial.print(temp);
   // Serial.print("; Humidity: ");
    //Serial.println(humid);

    nextUpdate = currMillis + 500;
}
