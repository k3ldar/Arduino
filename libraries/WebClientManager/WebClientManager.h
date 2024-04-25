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
#define DISCONNECT_TIMEOUT 2000
#define CONNECT_TIMEOUT 10000
#define MAX_CONNECT_FAILURES 5
#define MIN_STATUS_TIME 600

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
	int _wifiConnectFailures;
	unsigned long _nextWiFiConnect;
	unsigned long _nextRssi;
	unsigned long _nextStatus;

	long _lastRssi;
	
	char *_ssid;
	char *_pass;
	
	WiFiClient *_wifiClient;
	SendMessageCallback *_sendMessageCallback;
	
	void internalInitialize();
	void sendDebugStatus();
	bool connectToSocket(const unsigned long currMillis, const char *server, uint16_t port);
	void disconnectFromSocket();
	void wait(unsigned long currMillis, unsigned long waitTime);
public:
	WebClientManager();
	~WebClientManager();
	void initialize(SendMessageCallback *sendMesageCallbackr, int timeout, char *ssid, char *pass);

	void process(unsigned long currMillis);
	
	bool isWaiting(unsigned long currMillis);

	void connectToWiFi();
	void printWifiStatus();
	int getWiFiStatus();
	bool isWifiConnected();
	int wifiConnectFailures();
	int socketConnectFailures();
	String wifiStatus();
	String wifiConnectionStatus();
	void setTimeout(int timeout);
	long getRssi();
	bool canConnectToSocket(unsigned long currMillis);


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