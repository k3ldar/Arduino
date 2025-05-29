#ifndef MOVEMENTDETECTOR_H
#define MOVEMENTDETECTOR_H

#include <Arduino.h>

// Enum for movement states
enum State {
  IDLE,
  MOVED,
  STOPPED,
  DANGERCLOSE,
  OUTOFRANGE 
};

enum MovementDirection {
  NONE,
  FORWARD,
  BACKWARD
};

// Struct for configuration options
struct DetectorConfig {
  float movementThreshold = 1.5;         // cm
  int stabilityCheckCount = 3;           // how many consistent readings to consider stable
  float dangerCloseDistance = 25.0;      // cm
  float defaultBaseline = 100.0;          // fallback if baseline detection fails
  int sleepDelay = 100;                  // ms between updates
  float maximumDistance = 200;           // cm beyond this it won't be triggered
  int outOfRangeCheckCount = 2;         // number of inconsistent reading before out of range triggered
  int averageReadingCount = 3;         // number of readings to obtain before average is returned
  float dynamicThreshold = 0.01f;       // dynamic threshold percentage (lower for better sensitivity)
};

// Type alias for state change callback function
typedef void (*StateChangeCallback)(State oldState, State newState, float distance, MovementDirection oldDirection, MovementDirection newDirection);
typedef void (*ResetBaseLineCallback)();

class MovementDetector {
public:
  MovementDetector(int triggerPin, int echoPin);

  void begin(const DetectorConfig& cfg);
  void update();

  void setStateChangeCallback(StateChangeCallback cb);
  void setResetBaseLineCallback(ResetBaseLineCallback cb);
  const char* getStateString(State s);
  State getCurrentState() const;

private:
  int triggerPin;
  int echoPin;

  DetectorConfig config;

  State currentState = IDLE;
  State previousState = IDLE;

  MovementDirection currentDirection = NONE;
  MovementDirection previousDirection = NONE;

  float maxDistance = 400;
  float baselineDistance = 0;
  float previousDistance = 0;
  float lastValidDistance = 100.0;
  bool distanceValid;
  int stableCounter = 0;
  int outOfRangeCounter = 0;

  StateChangeCallback onStateChange = nullptr;
  ResetBaseLineCallback onResetBaseLine = nullptr;

  void changeState(State newState, MovementDirection newDirection);
  void resetBaseLine();
  float getDistance();
  bool withinDelta(float a, float b, float delta);
  bool initializeBaselineDistance(float& baseline, int maxRetries = 5, unsigned long retryDelayMs = 500);
};

#endif // MOVEMENTDETECTOR_H
