// MovementDetector Sample - https://github.com/k3ldar/Arduino/tree/main/MovementDetector
// Copyright © 2025, Simon Carter
// GPL-3.0 License

#include "MovementDetector.h"


#define SerialConnectDelay 200

#define SensorPowerPin D13
#define EchoPin D12
#define TriggerPin D11

#define Light5 D6
#define Light4 D5
#define Light3 D4
#define Light2 D3
#define Light1 D2

enum DangerLightPhase
{
  NO_FLASHING,
  FAST_FLASHING,
  SLOW_FLASHING
};

const int lightPins[] = { Light1, Light2, Light3, Light4, Light5 };
const float lightPinActivationDistance[] = { 190, 150, 110, 70, 40 };
const int numLights = sizeof(lightPins) / sizeof(lightPins[0]);
bool lightActive[] = { false, false, false, false, false };
bool isDangerFlashing = false;
unsigned long lastFlashTime = 0;
const unsigned long flashInterval = 100;  // milliseconds
const unsigned long sleepDelayNone = 0;
const unsigned long sleepDelayShort = 5;
const unsigned long sleepDelayMedium = 120;
const unsigned long sleepDelayLong = 250;

MovementDetector detector(TriggerPin, EchoPin);
float currentDistance = 400;
State currentState = IDLE;
unsigned long lastMovementTime = 0;
DangerLightPhase dangerLightPhase = NO_FLASHING;
unsigned long sleepDelay = sleepDelayShort;
const unsigned long noMovementTimeout = 10000; // 10 seconds after last movement
const unsigned long fastFlashDuration = 5000;  // 0–5 sec fast
const unsigned long slowFlashDuration = 8000;  // 5–10 sec slow
const unsigned long flashIntervalFast = 100;
const unsigned long flashIntervalSlow = 500;


void updateLight(int index, bool isOn)
{
  digitalWrite(lightPins[index], isOn ? HIGH : LOW);
  lightActive[index] = isOn;
}

void onStateChanged(State oldState, State newState, float distance, MovementDirection oldDirection, MovementDirection newDirection)
{
  Serial.print("State changed: ");
  Serial.print(detector.getStateString(oldState));
  Serial.print(" -> ");
  Serial.print(detector.getStateString(newState));
  Serial.print("; Distance: ");
  Serial.print(distance);
  Serial.print("; Old Direction: ");
  Serial.print(oldDirection);
  Serial.print("; New Direction: ");
  Serial.print(newDirection);

  currentState = newState;
  currentDistance = distance;

  if (newDirection == FORWARD || newDirection == BACKWARD) {
      lastMovementTime = millis();
  }

  if (oldState == DANGERCLOSE && newState != DANGERCLOSE)
  {
    isDangerFlashing = false;
    turnAllLightsOff();
  }

  switch (newState)
  {
    case IDLE: 
      Serial.println("🔄 Ready for next movement.");
      sleepDelay = sleepDelayMedium;
      return;
      
    case MOVED: 
      if (newDirection == FORWARD)
      {
        Serial.println("🔔 Movement detected ⬅️ forward");
      }
      else if (newDirection == BACKWARD)
      {
        Serial.println("🔔 Movement detected ➡️ backward");
      }
      else
      {
        Serial.println("🔔 Movement detected!");
      }
      sleepDelay = sleepDelayNone;
      break;

    case STOPPED: 
      Serial.println("✅ Movement stopped.");
      lastMovementTime = millis(); 
      sleepDelay = sleepDelayShort;
      return;

    case DANGERCLOSE:
      if (newDirection == FORWARD)
        Serial.println("🚨 DANGER CLOSE ⬅️ forward");
      else if (newDirection == BACKWARD)
        Serial.println("🚨 DANGER CLOSE ➡️ backward");
      else
        Serial.println("🚨 DANGER CLOSE!");
      
      isDangerFlashing = true;

      for (int i = 0; i < numLights; i++) {
        lightActive[i] = random(0, 2) == 1;
      }
      sleepDelay = sleepDelayNone;
      return;

    case OUTOFRANGE:
      Serial.println("❌ Distance out of range!");
      turnAllLightsOff();
      sleepDelay = sleepDelayLong;     

      return;
    
    default: 
      sleepDelay = sleepDelayLong;     
      Serial.println("Invalid Switch");
      return;
    
  }
}

void startupLightSequence()
{
  const int delayBetweenSteps = 100; // delay per LED step in ms
  const int cycleDuration = (numLights * 2 - 2) * delayBetweenSteps; // one full sweep (L->R->L)
  const int totalDuration = 4000; // total sequence time in ms
  const int repeatCycles = totalDuration / cycleDuration;

  for (int cycle = 0; cycle < repeatCycles; cycle++)
  {
    // Left to Right
    for (int i = 0; i < numLights; i++)
    {
      updateLight(i, true);
      delay(delayBetweenSteps);
      updateLight(i, false);
    }

    // Right to Left (excluding ends so it doesn't double-flash)
    for (int i = numLights - 2; i > 0; i--)
    {
      updateLight(i, true);
      delay(delayBetweenSteps);
      updateLight(i, false);
    }
  }

  turnAllLightsOff(); // reset before normal operation
}

void turnAllLightsOff()
{
  for (int i = 0; i < numLights; i++)
  {
    updateLight(i, false);
  }
}

void activateLightBasedOnDistance()
{
  for (int i = 0; i < numLights; i++)
  {
    bool isActive = currentDistance < lightPinActivationDistance[i];
    updateLight(i, isActive);
  }
}
void onResetBaseLine() {
  turnAllLightsOff();
}

void flashLights(unsigned long interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - lastFlashTime >= interval) {
    lastFlashTime = currentMillis;

    for (int i = 0; i < numLights; i++) {
      lightActive[i] = !lightActive[i];
      digitalWrite(lightPins[i], lightActive[i] ? HIGH : LOW);
    }
  }
}


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delayMicroseconds(200);
  }

  Serial.println("Serial Connected");

  // movement detector settings
  DetectorConfig config;
  config.movementThreshold = 1.5;       // cm
  config.stabilityCheckCount = 3;
  config.dangerCloseDistance = 30.0;    // cm
  config.defaultBaseline = 100.0;       // cm
  config.maximumDistance = 250;         // cm
  config.sleepDelay = 30;               // ms

  detector.setup(config);
  detector.setStateChangeCallback(onStateChanged);
  detector.setResetBaseLineCallback(onResetBaseLine);

  pinMode(SensorPowerPin, OUTPUT);
  digitalWrite(SensorPowerPin, HIGH);
  delay(500);


  for (int i = 0; i < numLights; i++)
  {
    pinMode(lightPins[i], OUTPUT);
  }
  
  startupLightSequence();
}

void loop()
{
  delay(sleepDelay);
  detector.update();

  if (currentState == DANGERCLOSE)
  {
    unsigned long elapsed = millis() - lastMovementTime;

    if (elapsed < fastFlashDuration)
    {
      dangerLightPhase = FAST_FLASHING;
      flashLights(flashIntervalFast);
    }
    else if (elapsed < fastFlashDuration + slowFlashDuration)
    {
      dangerLightPhase = SLOW_FLASHING;
      flashLights(flashIntervalSlow);
    }
    else
    {
      dangerLightPhase = NO_FLASHING;
      turnAllLightsOff();
    }
  }
  else 
  {
    dangerLightPhase = NO_FLASHING;

    unsigned long elapsed = millis() - lastMovementTime;

    if ((currentState != MOVED && currentState != DANGERCLOSE) && elapsed > noMovementTimeout)
    {
      turnAllLightsOff();
    }
    else
    {
      activateLightBasedOnDistance();
    }
  }
}