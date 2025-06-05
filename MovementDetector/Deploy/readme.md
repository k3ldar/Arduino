# MovementDetector

**MovementDetector** is a robust Arduino-compatible library designed to simplify movement detection using ultrasonic sensors like the HC-SR04. It enhances basic distance sensing by introducing **state tracking**, **movement direction detection**, **danger-close alerts**, and **custom callbacks**—ideal for interactive projects, safety systems, robotics, and more.

---

## Why Use MovementDetector?

While standard ultrasonic libraries only return raw distances, **MovementDetector** goes further:

- **Smart Movement Detection** – Determines if something is moving toward or away from the sensor.
- **Direction Awareness** – Easily detect forward or backward motion.
- **Danger Close Alerts** – Trigger events when objects come too close.
- **Stable State Transitions** – Handles jitter and false positives with configurable stability checks.
- **Simple API with Callbacks** – Respond to movement changes in real-time using custom functions.

---

## Features

- Supports any ultrasonic sensor compatible with `digitalWrite`/`pulseIn` (e.g., HC-SR04)
- Detects direction: `FORWARD`, `BACKWARD`, `STATIONARY`, or `UNKNOWN`
- State machine with transitions: `IDLE`, `MOVED`, `STOPPED`, `DANGERCLOSE`, `OUTOFRANGE`
- Adjustable sensitivity, distance thresholds, and stability check count
- Easy integration with existing projects

---

## Installation

You can install via the Arduino Library Manager once submitted (coming soon).

Or manually:

1. Clone or download this repository.
2. Copy the `MovementDetector` folder to your Arduino `libraries` directory.
3. Restart the Arduino IDE.

---

## State Machine Diagram


[IDLE] ---> [MOVED] ---> [STOPPED]
   |            |
   |            v
   |        [DANGERCLOSE]
   |
   +-------> [OUTOFRANGE]


## States
IDLE: No significant change
MOVED: Object is moving
STOPPED: Object has stopped moving
DANGERCLOSE: Object is within danger distance
OUTOFRANGE: Object is too far or not detected

## Directions
FORWARD: Object is approaching
BACKWARD: Object is retreating
STATIONARY: No movement detected
UNKNOWN: Initial state or indeterminate

## Configuration Options
Option	Purpose	Default
movementThreshold	Minimum distance change (cm) to detect	1.5
stabilityCheckCount	Number of consistent readings for a state	3
dangerCloseDistance	Distance (cm) to trigger DANGERCLOSE	30.0
defaultBaseline	Reference idle distance (cm)	100.0
maximumDistance	Maximum range to accept data (cm)	200.0

## License
This project is licensed under the GPL-3.0 license.

# Contributing
Feel free to fork and contribute with pull requests. Bug reports, suggestions, and feature ideas are always welcome!

