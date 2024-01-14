#include "WeatherStation.h"

WeatherStation::WeatherStation(const int tempSensorSignalPin, int rainSensorAnalogPin)
{
	_lm75 = new Generic_LM75;
	_tempSensorSignalPin = tempSensorSignalPin;
	_rainSensorAnalogPin = rainSensorAnalogPin;
	_currentTemperature = -66.0;
	_nextUpdateTime = millis() + 5000;
}

WeatherStation::~WeatherStation()
{
  delete(_lm75);
}

void WeatherStation::initialize(SendMessageCallback *sendMesageCallback, RFCommunicationManager *rfCommandMgr)
{
	_sendMessageCallback = sendMesageCallback;
	_rfCommandMgr = rfCommandMgr;
	Wire.begin();      
}

void WeatherStation::process()
{
	unsigned long currTime = millis();

	if (currTime > _nextUpdateTime)
	{
		readTemperatureSensor();
		readRainSensor();
		_nextUpdateTime = millis() + MillisBetweenSensorReading;
	}
}

void WeatherStation::readTemperatureSensor()
{
	_currentTemperature = _lm75->readTemperatureC();

	String updateTemp = "Current Temp: " +
	String(_currentTemperature);
	_sendMessageCallback(updateTemp, Information);

    char tempBuf[MAX_PACKET_DATA_SIZE];
    dtostrf(_currentTemperature, 0, 2, tempBuf);

	if (_rfCommandMgr != NULL && _rfCommandMgr->isInitialized())
    {
		  _rfCommandMgr->sendMessage(true, 500, tempBuf);
    }
	
    if (_sendMessageCallback != NULL)
    {
		_sendMessageCallback(String(tempBuf), Debug);
    }
    else
    {
		Serial.println("Not initialized");
    }
}

void WeatherStation::readRainSensor()
{
	int sensorValue = analogRead(_rainSensorAnalogPin);
	int outputValue = map(sensorValue, 0, 1023, 255, 0); // map the 10-bit data to 8-bit data
	String msg = "Rains sensor value: ";
	msg.concat(sensorValue);
	msg.concat("; Moisture Level: ");
	msg.concat(outputValue);
	_sendMessageCallback(msg, Debug);
}
