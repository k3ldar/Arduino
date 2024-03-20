#ifndef __WebClientManager__H
#define __WebClientManager__H

#include <Arduino.h>
#include <WiFiS3.h>
#include <ArduinoJson.h>


#include "Common.h"

#define MAXIMUM_FAILURES_TO_RECONNECT 5

class WebClientManager
{
private:
	int _wifiStatus;
	bool _socketConnected;
	int _connectResult;
	bool _getRequestSent;
	int _wifiTimeout;
	unsigned long _nextSocketConnectionRequest;
	int _socketConnectFailures;
	unsigned long _processCalledCount;

	char *_ssid;
	char *_pass;
	
	WiFiClient *_wifiClient;
	SendMessageCallback *_sendMessageCallback;
	
	void internalInitialize();
public:
	WebClientManager();
	~WebClientManager();
	void initialize(SendMessageCallback *sendMesageCallbackr, int timeout, char *ssid, char *pass);

	void process();

	void connectToWiFi();
	void printWifiStatus();
	int getWiFiStatus();
	bool isWifiConnected();
	int socketConnectFailures();

	bool get(const unsigned long currMillis, const char *server, uint16_t port, const char *path);
	bool getRequestSent();
	int getReadResponse(char *buffer, int bufferSize);
	bool getHasResponse();
	
	bool post(const char *server, const char *path);
	
	
	JsonResponse htmlParseBody(const String& data);
};

#endif