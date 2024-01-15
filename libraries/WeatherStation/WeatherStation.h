#ifndef __WeatherStation__
#define __WeatherStation__

#include <Arduino.h>
#include <Temperature_LM75_Derived.h>
#include <dht11.h>
#include "RFCommunicationManager.h"
#include "Common.h"

#define MillisBetweenSensorReading 1500;

class WeatherStation
{
private:
	Generic_LM75 *_lm75;
	dht11 *_dht11;
	int _dht11SensorSignalPin;
	double _currentTemperature;
	float _tempCelsius;

	int _rainSensorAnalogPin;

	unsigned long _nextUpdateTime;
	void readTemperatureSensor();
	void readRainSensor();
	void readDHT11Sensor();

	SendMessageCallback *_sendMessageCallback;
	RFCommunicationManager *_rfCommandMgr;
public:
	WeatherStation(int tempSensorSignalPin, int rainSensorAnalogPin);
	~WeatherStation();
	void initialize(SendMessageCallback *sendMesageCallback, RFCommunicationManager *rfCommandMgr);
	void process();
};


#endif