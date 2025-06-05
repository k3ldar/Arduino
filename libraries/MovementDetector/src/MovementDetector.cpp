// MovementDetector v1.0 - https://github.com/k3ldar/MovementDetector
// Copyright © 2025, Si Carter
// MIT License

#include "MovementDetector.h"

#define MinimumDistance 0.0
#define MaximumDistance 400.0
#define TriggerLowDelay 2 // µs LOW before trigger
#define TriggerHighDelay 10 // µs HIGH for trigger pulse


MovementDetector::MovementDetector(int triggerPin, int echoPin)
  : triggerPin(triggerPin), echoPin(echoPin) {}

void MovementDetector::setup(const DetectorConfig& cfg)
{
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  config = cfg;

  if (!initializeBaselineDistance(baselineDistance)) {
    Serial.println("⚠️ Failed to get valid baseline distance after retries.");
    baselineDistance = config.defaultBaseline;
  } else {
    Serial.print("Baseline distance set to: ");
    Serial.println(baselineDistance);
  }

  maxDistance = config.maximumDistance < MaximumDistance ? config.maximumDistance : MaximumDistance;
}

void MovementDetector::update()
{
  float distance = getDistance();

  if (!distanceValid) {
    resetBaseLine();
    return;
  }

  if (distance < MinimumDistance || distance > maxDistance) {
    if (++outOfRangeCounter > config.outOfRangeCheckCount) {
      changeState(OUTOFRANGE, NONE);
      previousDistance = lastValidDistance;
      return; 
    }
  } else {
    outOfRangeCounter = 0;
  }

  MovementDirection newDirection = NONE;
  float dynamicThreshold = max(config.movementThreshold, distance * config.dynamicThreshold);

  if (!withinDelta(distance, previousDistance, dynamicThreshold)) {
    newDirection = (distance < previousDistance) ? FORWARD : BACKWARD;
    previousDistance = distance; 
  }

  if (distance < config.dangerCloseDistance) {
    changeState(DANGERCLOSE, newDirection);
    baselineDistance = distance;
    return;
  }
  
  switch (currentState) {
    case IDLE:
       if (!withinDelta(distance, baselineDistance, dynamicThreshold)) {
        changeState(MOVED, newDirection);
        
        stableCounter = 0;
        previousDistance = distance;
      }
      break;

    case MOVED:
      if (withinDelta(distance, previousDistance, dynamicThreshold)) {
        stableCounter++;
        if (stableCounter >= config.stabilityCheckCount) {
          changeState(STOPPED, newDirection);
          baselineDistance = distance;
        }
      } else {
        stableCounter = 0;        
        previousDistance = distance;
      }
      break;

    case STOPPED:
      stableCounter = 0;
      changeState(IDLE, NONE);
      break;

    case DANGERCLOSE:
      if (!withinDelta(distance, config.dangerCloseDistance, dynamicThreshold)) {
        baselineDistance = distance;
        stableCounter = 0;
        changeState(IDLE, newDirection);
      }
      break;

    case OUTOFRANGE:
        changeState(IDLE, newDirection);
        break;
  }

  delay(config.sleepDelay);
}

void MovementDetector::setStateChangeCallback(StateChangeCallback cb) {
  onStateChange = cb;
}

void MovementDetector::setResetBaseLineCallback(ResetBaseLineCallback cb) {
  onResetBaseLine = cb;
}

const char* MovementDetector::getStateString(State s) {
  switch (s) {
    case IDLE: return "IDLE";
    case MOVED: return "MOVED";
    case STOPPED: return "STOPPED";
    case DANGERCLOSE: return "DANGERCLOSE";
    case OUTOFRANGE: return "OUTOFRANGE";
    default: return "UNKNOWN";
  }
}

State MovementDetector::getCurrentState() const {
  return currentState;
}

void MovementDetector::resetBaseLine(){
  Serial.println("⚠️ All readings invalid. Resetting baseline.");
  baselineDistance = lastValidDistance;
  currentState = IDLE;

  if (onResetBaseLine)
    onResetBaseLine();
}

void MovementDetector::changeState(State newState, MovementDirection newDirection) {
  if (newState != currentState || newDirection != currentDirection) {
    previousState = currentState;
    currentState = newState;
    previousDirection = currentDirection;
    currentDirection = newDirection;
    
    if (newState == IDLE || newState == STOPPED) {
      currentDirection = NONE;
    }

    if (onStateChange) {
      onStateChange(previousState, currentState, baselineDistance, previousDirection, currentDirection);
    }
  }
}

float MovementDetector::getDistance() {
  float total = 0;
  int validReadings = 0;

  for (int i = 0; i < config.averageReadingCount; i++) {
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000); // Timeout at 30ms (~500cm)

    if (duration == 0) {
      Serial.println("⚠️ No echo received (timeout).");
      continue; // Skip this reading
    }

    float distance = duration * 0.034 / 2;

    if (distance <= MinimumDistance || distance > maxDistance) {
      Serial.print("⚠️ Ignoring out-of-range reading: ");
      Serial.println(distance);
      continue;
    }

    total += distance;
    validReadings++;
    delay(50); // Small delay between readings to avoid crosstalk
  }

  if (validReadings > 0) {
    float average = total / validReadings;
    lastValidDistance = average;
    distanceValid = true;
    return average;
  } else {
    Serial.println("⚠️ Using last known good distance.");
    distanceValid = false;
    return lastValidDistance;
  }
}

bool MovementDetector::withinDelta(float a, float b, float delta) {
  return abs(a - b) <= delta;
}

bool MovementDetector::initializeBaselineDistance(float& baseline, int maxRetries, unsigned long retryDelayMs) {
  int retries = 0;
  float dist;
  while (retries < maxRetries) {
    dist = getDistance();
    if (dist > MinimumDistance && dist <= MaximumDistance) {
      baseline = dist;
      return true;
    }
    retries++;
    delay(retryDelayMs);
  }
  return false;
}
