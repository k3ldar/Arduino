
#include <StepperManager.h>

const long TopPosition = 1000000;
const long BottomPosition = -1000000;
const float MaxSpeed = 200;
const float Acceleration = 50;
const float Speed = 100;



StepperManager::StepperManager(SerialCommandManager* cmdManager, ezButton limitTop, ezButton limitBottom, int stepperPin1, String name)
{
	_commandManager = cmdManager;
	_isStopped = true;
	_initialized = 0;
	_name = name;
	_stepperPin1 = stepperPin1;
	_stepperPin2 = _stepperPin1++;
	_stepperPin3 = _stepperPin2++;
	_stepperPin4 = _stepperPin3++;
	_nextPosition = 0;
	_direction = None;
	_speed = Speed;
	_acceleration = Acceleration;
	
	_limitSwitchTop = limitTop;
	_limitSwitchBottom = limitBottom;

	AccelStepper stepper(AccelStepper::FULL4WIRE, _stepperPin1, _stepperPin2, _stepperPin3, _stepperPin4);
	_stepper = stepper;
}


void StepperManager::setStepperIdle() 
{
	_commandManager->sendDebug("stepperIdle", _name);
	_isStopped = true;
	digitalWrite(_stepperPin1, LOW);
	digitalWrite(_stepperPin2, LOW);
	digitalWrite(_stepperPin3, LOW);
	digitalWrite(_stepperPin4, LOW);
}

void StepperManager::setStepperActive() 
{
	_commandManager->sendDebug("stepperActive", _name);
	setSpeed();
	_isStopped = false;
	digitalWrite(_stepperPin1, HIGH);
	digitalWrite(_stepperPin2, HIGH);
	digitalWrite(_stepperPin3, HIGH);
	digitalWrite(_stepperPin4, HIGH);
}

void StepperManager::stop()
{
	_isStopped = true;
	_commandManager->sendDebug("Stop Issued", _name);
	setStepperIdle();
}

void StepperManager::setSpeed()
{
	_commandManager->sendDebug("stepperSettingSpeed " + String(_speed), _name);
	_stepper.setMaxSpeed(MaxSpeed);
	_stepper.setAcceleration(_acceleration);
	_stepper.setSpeed(_speed);
}

void StepperManager::setup()
{
	_limitSwitchTop.setDebounceTime(50);
	_limitSwitchBottom.setDebounceTime(50);
	_stepper.enableOutputs();
	setSpeed();

	_stepper.setMaxSpeed(200.0);
    _stepper.setAcceleration(100.0);
    _stepper.moveTo(50);
	_initialized++;
}
	
void StepperManager::process()
{
	_limitSwitchTop.loop();
	_limitSwitchBottom.loop();
	
	if (_limitSwitchTop.isPressed())
	{
		_commandManager->sendDebug(" Top limit switch reached", _name);
		stop();
	}

	if (_limitSwitchBottom.isPressed())
	{
		_commandManager->sendDebug(_name + " Bottom limit switch reached", _name);
		stop();
	}

	if (!_isStopped) 
	{
		if (_stepper.distanceToGo() == 0) 
		{ 
			_commandManager->sendDebug(_name + " Stepper SettingDistanceToGo", _name);
			//_stepper.setCurrentPosition(0);
			setSpeed();
			_stepper.moveTo(_nextPosition);       
		}
		
		_stepper.run(); // MUST be called in loop() function
	}
}

bool StepperManager::processMessage()
{
	String command = _commandManager->getCommand();

	if (!command.startsWith("SM-"))
		return false;
	
    if (command == "SM-D")
    {
		if (_isStopped)
		{
			//_stepper.setCurrentPosition(0);
			_stepper.moveTo(TopPosition);       
			_nextPosition = TopPosition;
			setStepperActive();
			_commandManager->sendCommand("SM-D", "Moving Down", _name);
		}
		else
		{
			_commandManager->sendCommand("SM-D", "Already moving", _name);
		}
	  
		return true;
    }
    else if (command == "SM-U")
    {
		if (_isStopped)
		{
			//_stepper.setCurrentPosition(0);   
			_stepper.moveTo(BottomPosition);       
			_nextPosition = BottomPosition;
			setStepperActive();
			_commandManager->sendCommand("SM-U", "Moving Up", _name);
		}
		else
		{
			_commandManager->sendCommand("SM-U", "Already Moving", _name);
		}

		return true;
	}
	else if (command == "SM-S")
	{
		if (_isStopped)
		{
			_commandManager->sendCommand("SM-S", "Not Running", _name);
		}
		else
		{
			_commandManager->sendCommand("SM-S", "Stop Issued", _name);
			setStepperIdle();
		}

		return true;
	}
	else if (command == "SM-P")
	{
		_commandManager->sendCommand("SM-P", String(_stepper.currentPosition()), _name);
		return true;
	}
	else if (command == "SM-I")
	{
		_commandManager->sendCommand("SM-I", String(!_isStopped), _name);
		return true;
	}
	else if (command == "SM-DBG")
	{
		_commandManager->sendDebug("Stepper Initialize Count " + String(_initialized), _name);
		_commandManager->sendDebug("Stepper Active " + String(_isStopped), _name);
		_commandManager->sendDebug("Stepper Next Position " + String(_nextPosition), _name);
		_commandManager->sendDebug("Stepper Current Position " + String(_stepper.currentPosition()), _name);
		_commandManager->sendDebug("Stepper Distance to go " + String(_stepper.distanceToGo()), _name);
		_commandManager->sendDebug("Stepper Speed " + String(_stepper.speed()), _name);
		_commandManager->sendDebug("Stepper Max Speed " + String(_stepper.maxSpeed()), _name);
		_commandManager->sendDebug("Stepper Acceleration " + String(_acceleration), _name);
		return true;
	}

	return false;
}