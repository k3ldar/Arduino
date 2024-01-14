// z = 3
// y = 2
// x = 1

const int JoystickNoInput = 500; 
const int joystickMovingDown = 400;
const int joystickMovingUp = 600;

// Stepper Motor Y
const int stepPin = 3; //Y.STEP
const int dirPin = 6; // Y.DIR

// joystick pins
const int JoystickDownPin = A0; 
const int JoystickUpPin = A2;
 
const unsigned long NumberOfSteps = 100;
const unsigned long Speed = 600;

unsigned long prevStepMicros = 0;
unsigned long microsBetweenSteps = 60L * 1000L * 1000L / NumberOfSteps / Speed;

int lastJoystickPosition = 0;
bool dirctionPinSet = false;

void setup() {
    // Sets the two pins as Outputs
    Serial.begin(9600); 
    pinMode(stepPin, OUTPUT); 
    pinMode(dirPin, OUTPUT);
    pinMode(JoystickDownPin, INPUT); 
    pinMode(JoystickUpPin, INPUT); 
}


void loop() 
{
    prevStepMicros = micros();
    joystick(); 
}

void joystick()
{
    int JoystickPosition = analogRead(JoystickDownPin);

    if (JoystickPosition > joystickMovingDown && JoystickPosition < joystickMovingUp)
      JoystickPosition = JoystickNoInput;
      
    if (JoystickPosition != lastJoystickPosition)
    {
        lastJoystickPosition = JoystickPosition;
        Serial.print("Joystick Position:"); 
        Serial.println(lastJoystickPosition);
    }

    // to stop the stepper motor
    if (JoystickPosition == JoystickNoInput)
    {
        dirctionPinSet = false;
        
        return;
    }

    if (JoystickPosition > joystickMovingUp)
    {
        moveUp(JoystickPosition);
    }

    if (JoystickPosition < joystickMovingDown)
    {
        moveDown(JoystickPosition);
    }
}

void moveUp(int joystickPosition)
{
    if (!dirctionPinSet)
    {
        Serial.println("Turning Step Pin On (Up)");
        digitalWrite(dirPin, HIGH);
        dirctionPinSet = true;
    }
    
    takeSingleStep();
}

void moveDown(int joystickPosition)
{      
    if (!dirctionPinSet)
    {
        Serial.println("Turning Step Pin On (Down)");
        digitalWrite(dirPin, LOW);
        dirctionPinSet = true;
    }

    takeSingleStep();
}

void takeSingleStep()
{
    unsigned long now = micros();
    
    if (now - prevStepMicros >= microsBetweenSteps) 
    {
        prevStepMicros = now;
        digitalWrite(stepPin, HIGH);
        digitalWrite(stepPin, LOW);
        Serial.println("moving");
    }
}
