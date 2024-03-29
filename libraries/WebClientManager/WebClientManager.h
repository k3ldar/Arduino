#ifndef __WebClientManager__H
#define __WebClientManager__H

#include <Arduino.h>
#include <WiFiS3.h>
#include <ArduinoJson.h>


#include "Common.h"

#define MAXIMUM_FAILURES_TO_RECONNECT 5
#define SOCKET_CONNECT_DELAY 1000

#define GET_REQUEST_PENDING -1
#define POST_REQUEST_PENDING -2
#define GET_REQUEST_TIMEOUT -3
#define POST_REQUEST_TIEMOUT -4

#define REQUEST_TIMEOUT_MS 10000

static const char RESET[] = "AT+RESET\n";


class WebClientManager
{
private:
	int _wifiStatus;
	bool _socketConnected;
	int _connectResult;
	bool _getRequestSent;
	unsigned long _getRequestTimeoutTime;
	bool _postRequestSent;
	unsigned long _postRequestTimeoutTime;
	int _wifiTimeout;
	unsigned long _nextSocketConnectionRequest;
	int _socketConnectFailures;

	char *_ssid;
	char *_pass;
	
	WiFiClient *_wifiClient;
	SendMessageCallback *_sendMessageCallback;
	
	void internalInitialize();
	void sendDebugStatus();
	void connectToSocket(const unsigned long currMillis, const char *server, uint16_t port);
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
	String wifiStatus();

	bool get(const unsigned long currMillis, const char *server, uint16_t port, const char *path);
	bool getRequestSent();
	int getReadResponse(char *buffer, int bufferSize);
	bool getHasResponse();
	
	bool post(const unsigned long currMillis, const char *server, uint16_t port, const char *path);
	bool postRequestSent();
	int postReadResponse(char *buffer, int bufferSize);
	bool postHasResponse();
	
	
	JsonResponse htmlParseJsonBody(const String& data);
};

#endif