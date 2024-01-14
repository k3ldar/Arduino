#ifndef __StepperManager__
#define __StepperManager__

#include <AccelStepper.h>
#include <SerialCommandManager.h>
#include <ezButton.h>
#include <String.h>


enum Direction
{
  None,
  Forward,
  Backward
};


class StepperManager
{
private:
	SerialCommandManager* _commandManager;
	Direction _direction = None;
	bool _isStopped;
	bool _finalizeComms;
	String _name;
	int _stepperPin1; 
	int _stepperPin2; 
	int _stepperPin3; 
	int _stepperPin4;
	long _nextPosition;
	int _initialized;
	float _speed = 100.0;
	float _acceleration = 50.0;
	AccelStepper _stepper = NULL;
	ezButton _limitSwitchTop = 0;
	ezButton _limitSwitchBottom = 0;
	void setStepperActive();
	void setStepperIdle();
	void stop();
	void setSpeed();
public:
	StepperManager(SerialCommandManager* cmdManager, ezButton limitTop, ezButton limitBottom, int stepperPin1, String name = "Stepper");
	void setup();
	void process();
	bool processMessage();
};

#endif