#include "WeatherStation.h"

WeatherStation::WeatherStation(const int dht11SensorSignalPin, int rainSensorAnalogPin)
{
	_lm75 = new Generic_LM75;
	_dht11 = new dht11;
	_dht11SensorSignalPin = dht11SensorSignalPin;
	_rainSensorAnalogPin = rainSensorAnalogPin;
	_currentTemperature = -66.0;
	_nextUpdateTime = millis() + 5000;
}

WeatherStation::~WeatherStation()
{
  delete(_lm75);
  delete(_dht11);
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
		readDHT11Sensor();
		_nextUpdateTime = millis() + MillisBetweenSensorReading;
	}
}

void WeatherStation::readDHT11Sensor()
{
	_dht11->read(_dht11SensorSignalPin);

	Serial.print("Humidity: ");
	Serial.print(_dht11->humidity);
	Serial.print("; Temp: ");
	Serial.println(_dht11->temperature);
	float humidity = (float)_dht11->humidity;
	float tempC = (float)_dht11->temperature;
	
	if (_rfCommandMgr != NULL && _rfCommandMgr->isInitialized())
    {
		char tempCBuf[MAX_PACKET_DATA_SIZE];
		dtostrf(tempC, 0, 2, tempCBuf);
		_rfCommandMgr->sendMessage(true, 502, tempCBuf);
		
		char humidBuf[MAX_PACKET_DATA_SIZE];
		dtostrf(humidity, 0, 2, humidBuf);
		_rfCommandMgr->sendMessage(true, 503, humidBuf);
    }
	
	String tempHum = "Temp: ";
	tempHum.concat(tempC);
	tempHum.concat("(C); Humidity: ");
	tempHum.concat(humidity);
	tempHum.concat("%");
	_sendMessageCallback(tempHum, Information);
}

void WeatherStation::readTemperatureSensor()
{
	_currentTemperature = _lm75->readTemperatureC();

	String updateTemp = "Current Temp: " +
	String(_currentTemperature);
	_sendMessageCallback(updateTemp, Information);

	if (_rfCommandMgr != NULL && _rfCommandMgr->isInitialized())
    {
		char tempBuf[MAX_PACKET_DATA_SIZE];
		dtostrf(_currentTemperature, 0, 2, tempBuf);

		_rfCommandMgr->sendMessage(true, 500, tempBuf);
		  
		if (_sendMessageCallback != NULL)
		{
			_sendMessageCallback(String(tempBuf), Debug);
		}
    }
}

void WeatherStation::readRainSensor()
{
	int sensorValue = analogRead(_rainSensorAnalogPin);
	int outputValue = map(sensorValue, 0, 1023, 255, 0); // map the 10-bit data to 8-bit data
	
	char tempBuf[MAX_PACKET_DATA_SIZE];
	sprintf(tempBuf, "%d", outputValue);
	
	if (_rfCommandMgr != NULL && _rfCommandMgr->isInitialized())
	{
		_rfCommandMgr->sendMessage(true, 501, tempBuf);
	}

    if (_sendMessageCallback != NULL)
    {
		String msg = "Rains sensor value: ";
		msg.concat(outputValue);
		_sendMessageCallback(msg, Information);
	}
}
