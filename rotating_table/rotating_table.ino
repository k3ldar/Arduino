#include <AccelStepper.h>

#define PIN_AUTO_IN D6
#define PIN_FOOT_RIGHT D5
#define PIN_FOOT_LEFT D4
#define PIN_MOTOR_PWM D3
#define PIN_MOTOR_RIGHT D2
#define PIN_MOTOR_LEFT D1

#define SPEED_DEFAULT 30
#define SPEED_UPDATE_MILLIS 700

#define DEFAULT_DELAY 50
#define LONG_PRESS 800
#define RESET_SPEED 3000

bool directionClockWise = true;
byte motorSpeedPercent = SPEED_DEFAULT;
bool motorRunning = false;
bool lastAutomatic = false;
bool leftPedalPressed = false;
bool rightPedalPressed = false;
unsigned long nextSpeedCheck = 0;
unsigned long startLeftPedalPressed = 0;
unsigned long startRightPedalPressed = 0;

void setup()
{
    Serial.begin(115200);
    while (!Serial);

    Serial.println("Configuring Turn Table");

    pinMode(PIN_AUTO_IN, INPUT_PULLUP);
    pinMode(PIN_FOOT_LEFT, INPUT_PULLUP);
    pinMode(PIN_FOOT_RIGHT, INPUT_PULLUP);
    pinMode(PIN_MOTOR_LEFT, OUTPUT);
    pinMode(PIN_MOTOR_RIGHT, OUTPUT);

    MotorSpeed(12);
}

void loop()
{
    unsigned long currMillis = millis();
    bool rightPedalRelease = false;
    bool leftPedalRelease = false;

    unsigned long rightPedalPressMillis = 0;
    unsigned long leftPedalPressMillis = 0;

    if (IsRightPedalPressed())
    {
        if (!rightPedalPressed)
        {
            rightPedalPressed = true;
            startRightPedalPressed = currMillis;
        }
    }
    else
    {
        if (rightPedalPressed)
        {
            rightPedalRelease = true;
            rightPedalPressed = false;
            rightPedalPressMillis = currMillis - startRightPedalPressed;
            startRightPedalPressed = 0;
        }
    }
    
    if (IsLeftPedalPressed())
    {
        if (!leftPedalPressed)
        {
            leftPedalPressed = true;
            startLeftPedalPressed = currMillis;
        }
    }
    else
    {
        if (leftPedalPressed)
        {
            leftPedalRelease = true;
            leftPedalPressed = false;
            leftPedalPressMillis = currMillis - startLeftPedalPressed;
            startLeftPedalPressed = 0;
        }
    }

    if (leftPedalPressed && rightPedalPressed && 
        (currMillis - startLeftPedalPressed) > RESET_SPEED && 
        (currMillis - startRightPedalPressed) > RESET_SPEED &&
        motorSpeedPercent != SPEED_DEFAULT)
    {
        MotorSpeed(SPEED_DEFAULT);
    }

    if (IsAutomatic())
    {
        MotorRun();

        if (rightPedalRelease && nextSpeedCheck < currMillis)
        {
            nextSpeedCheck = currMillis + SPEED_UPDATE_MILLIS;
            MotorIncreaseSpeed();
        }
        else if (leftPedalRelease && nextSpeedCheck < currMillis)
        {
            nextSpeedCheck = currMillis + SPEED_UPDATE_MILLIS;
            MotorDecreaseSpeed();
        }
    }
    else
    {
        if (leftPedalRelease && rightPedalPressed)
        {
            if (leftPedalPressMillis > LONG_PRESS)
            {
                MotorIncreaseSpeed();
            }
            else
            {
                MotorDecreaseSpeed();
            }

            Serial.println("left release right pressed");
        }
        else if (rightPedalRelease && leftPedalPressed)
        {
            if (rightPedalPressMillis > LONG_PRESS)
            {
                MotorIncreaseSpeed();
            }
            else
            {
                MotorDecreaseSpeed();
            }

            Serial.println("right release left pressed");
        }

        if (rightPedalPressed)
        {
            directionClockWise = true;
            MotorRun();
        }
        else if (leftPedalPressed)
        {
            directionClockWise = false;
            MotorRun();
        }
        else
        {
            MotorStop();    
            startLeftPedalPressed = 0;
            startRightPedalPressed = 0;
        }
    }

    delay(DEFAULT_DELAY);
}

bool IsAutomatic()
{
  bool result = digitalRead(PIN_AUTO_IN) == HIGH;

  if (result != lastAutomatic)
  {
    lastAutomatic = result;
    motorSpeedPercent = SPEED_DEFAULT;
    directionClockWise = true;

    if (lastAutomatic)
    {
      Serial.println("automatic");
      MotorSpeed(SPEED_DEFAULT);
    }
    else
    {
      Serial.println("manual");
      MotorStop();
    }
  }

  return result;
}

bool IsLeftPedalPressed()
{
    return digitalRead(PIN_FOOT_LEFT) == LOW;
}

bool IsRightPedalPressed()
{
    return digitalRead(PIN_FOOT_RIGHT) == LOW;
}

void MotorIncreaseSpeed()
{
    motorSpeedPercent++;
    MotorSpeed(motorSpeedPercent);
}

void MotorDecreaseSpeed()
{
    motorSpeedPercent--;
    MotorSpeed(motorSpeedPercent);
}

void MotorSpeed(byte percent)
{
    if (percent < 1)
        percent = 1;

    if (percent > 100)
        percent = 100;

    motorSpeedPercent = percent;
    analogWrite(PIN_MOTOR_PWM, percent * 2.5);
    Serial.print("speed updated: ");
    Serial.print(motorSpeedPercent);
    Serial.println("%");
}

void MotorRun()
{
    if (motorRunning)
        return;

    motorRunning = true;

    if (directionClockWise)
    {
        digitalWrite(PIN_MOTOR_LEFT, LOW);
        digitalWrite(PIN_MOTOR_RIGHT, HIGH);
    }
    else
    {
        digitalWrite(PIN_MOTOR_LEFT, HIGH);
        digitalWrite(PIN_MOTOR_RIGHT, LOW);
    }
}

void MotorStop()
{
    if (!motorRunning)
        return;

    motorRunning = false;
    digitalWrite(PIN_MOTOR_LEFT, LOW);
    digitalWrite(PIN_MOTOR_RIGHT, LOW);
}



