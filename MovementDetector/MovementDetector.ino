// MovementDetector Sample - https://github.com/k3ldar/Arduino/tree/main/MovementDetector
// Copyright © 2025, Simon Carter
// GPL-3.0 License

#include "MovementDetector.h"


#define SerialConnectDelay 200

#define SensorPowerPin D13
#define EchoPin D12
#define TriggerPin D11

#define Light6 D7
#define Light5 D6
#define Light4 D5
#define Light3 D4
#define Light2 D3
#define Light1 D2

const float LightFlashDistance = 100.0;

MovementDetector detector(TriggerPin, EchoPin);
bool isLightOn = false;
bool isLightBlink = false;
bool canBlink = true;
unsigned long lightOffTime = 0;
float currentDistance = 400;
State currentState = IDLE;

void updateLight(bool isOn)
{
  if (isOn)
  {
    digitalWrite(Light1, HIGH);
    isLightOn = true;
  }
  else
  {
    digitalWrite(Light1, LOW);
    isLightOn = false;
  }
}

void onStateChanged(State oldState, State newState, float distance, MovementDirection oldDirection, MovementDirection newDirection) {
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

  if (oldState == DANGERCLOSE && newState != DANGERCLOSE)
  {
    updateLight(false);
  }

  switch (newState)
  {
    case IDLE: 
      Serial.println("🔄 Ready for next movement.");
      return;
      
    case MOVED: 
      
      if (newDirection == FORWARD)
      {
        if (distance <= LightFlashDistance && canBlink)
        {
          updateLight(true);
          isLightBlink = true;
          lightOffTime = millis() + 500;
          canBlink = false;
        }
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

      break;

    case STOPPED: 
      Serial.println("✅ Movement stopped.");
      return;

    case DANGERCLOSE:
      if (newDirection == FORWARD)
        Serial.println("🚨 DANGER CLOSE ⬅️ forward");
      else if (newDirection == BACKWARD)
        Serial.println("🚨 DANGER CLOSE ➡️ backward");
      else
        Serial.println("🚨 DANGER CLOSE!");
      
      updateLight(true);

      return;

    case OUTOFRANGE:
      Serial.println("❌ Distance out of range!");
      updateLight(false);

      return;
    
    default: 
      Serial.println("Invalid Switch");
      return;
    
  }
}

void onResetBaseLine() {
  updateLight(false);
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

  detector.setup(config);
  detector.setStateChangeCallback(onStateChanged);
  detector.setResetBaseLineCallback(onResetBaseLine);

  pinMode(SensorPowerPin, OUTPUT);
  digitalWrite(SensorPowerPin, HIGH);
  delay(500);

  pinMode(Light1, OUTPUT);
  updateLight(false);
}

void loop() {
  if (!canBlink && currentDistance > (LightFlashDistance + 5))
  {
    canBlink = true;
  }

  if (isLightOn && isLightBlink && millis() > lightOffTime)
  {
    isLightBlink = false;

    if (currentState != DANGERCLOSE)
    {
      updateLight(false);
    }
  }
  detector.update();
}
