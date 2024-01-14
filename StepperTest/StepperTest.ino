//#include <ESP8266WiFi.h>
#include <SerialCommandManager.h>
#include <StepperManager.h>


const int RadioPinCE = 51;
const int RadioPinCSN = 53;
const byte RadioAddress[6] = "00001";


SerialCommandManager commandMgr(commandReceived, ';', ':', '=', 50, 200);

ezButton leftTopLimit(2);
ezButton leftBottomLimit(3);
//2 Top limit switch and 3 bottom limit switch, pins 4, 5, 6 and 7 
StepperManager stepperMgrLeft(&commandMgr, leftTopLimit, leftBottomLimit, 4, "Left Stepper");

ezButton rightTopLimit(12);
ezButton rightBottomLimit(13);
//12 Top limit switch and 13 bottom limit switch, pins 8, 9, 10, 11
StepperManager stepperMgrRight(&commandMgr, rightTopLimit, rightBottomLimit, 8, "Right Stepper");


//RF24 radio(RadioPinCE, RadioPinCSN);
/*
void readRFCommands()
{
  if (radio.available())
  {
    char text[32] = "";
    radio.read(&text, sizeof(text));

    if (text[0] != '\0')
      Serial.println(text);
  }
}*/

void commandReceived()
{
  if (commandMgr.isTimeout())
  {
    Serial.println("message timed out");
  }
  else
  {
    if (stepperMgrLeft.processMessage())
      if (stepperMgrRight.processMessage())
        return;

    commandMgr.sendCommand("ERR", "UNKNOWN", "*");
  }
}

void setup() 
{
  //radio.begin();
  //radio.openWritingPipe(RadioAddress);
  //radio.setPALevel(RF24_PA_MIN);
  stepperMgrLeft.setup();
  stepperMgrRight.setup();
  Serial.begin(115200);
}

long _lastPosition;
void loop() 
{
  commandMgr.readCommands();
  //readRFCommands();
  
  stepperMgrLeft.process();
  stepperMgrRight.process();

  YIELD;
}
