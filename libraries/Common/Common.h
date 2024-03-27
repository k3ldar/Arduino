#ifndef __Common__H
#define __Common__H


enum MessageType
{
	Error,
	Debug,
	Information
};

enum DeviceType
{
	None = 0,
	WeatherStation = 1,
	WaterPump = 2,
	TemperatureSensor = 3,
};

typedef void SendMessageCallback(String message, MessageType messageType);


struct JsonResponse
{
	bool success;
	String json;
};

#endif