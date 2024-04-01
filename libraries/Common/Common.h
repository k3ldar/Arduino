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
	DeviceNone = 0,
	DeviceWeatherStation = 1,
	DeviceWaterPump = 2,
	DeviceTemperatureSensor = 3,
};

typedef void SendMessageCallback(String message, MessageType messageType);


struct JsonResponse
{
	bool success;
	String json;
};

#endif