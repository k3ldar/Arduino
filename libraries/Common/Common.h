#ifndef __Common__H
#define __Common__H


#include <Arduino.h>
#define WEATHER_STATION_
#define WATER_PUMP
#define DEBUG_
#define RELEASE

#if defined(WEATHER_STATION)
#define LED_WEATHER_STATION
#endif

#if defined(WATER_PUMP)
#define LED_WATER_PUMP
#endif

#define WIFI_FAILURES_FOR_RESTART 12
#define FAIL_REPORTING_MS 1000


static const char RESET_WIFI[] = "AT+RESET\r\n";


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

extern uint8_t _ledFrame[8][12];

String combineStrings(const char* format, ...);

#endif