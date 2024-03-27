#include "WebClientManager.h"

WebClientManager::WebClientManager()
{
	_connectResult = -1;
	_nextSocketConnectionRequest = 0;
	_socketConnectFailures = 0;
	_getRequestSent = false;
}

WebClientManager::~WebClientManager()
{
	delete(_wifiClient);
}

void WebClientManager::initialize(SendMessageCallback *sendMesageCallback, int timeout, char *ssid, char *pass)
{
	_sendMessageCallback = sendMesageCallback;
	_ssid = ssid;
	_pass = pass;
	_wifiTimeout = timeout;
	
	// check for the WiFi module:
	if (WiFi.status() == WL_NO_MODULE)
	{
		sendMesageCallback("Communication with WiFi module failed!", Information);
	}

	String fv = WiFi.firmwareVersion();
	
	if (fv < WIFI_FIRMWARE_LATEST_VERSION)
	{
		sendMesageCallback("Please upgrade the firmware", Information);
	}
	
	internalInitialize();
}

void WebClientManager::process()
{
	if (_wifiStatus == WL_CONNECTING)
	{
		_wifiStatus = WiFi.isConnected();
		printWifiStatus();
	}
	else if (_wifiStatus == WL_CONNECT_FAILED)
	{
		connectToWiFi();
	}
}

void WebClientManager::connectToWiFi()
{
  // attempt to connect to WiFi network:
  String connectMessage = "Attempting to connect to SSID: ";
  connectMessage.concat(_ssid);
  _sendMessageCallback(connectMessage, Information);
  
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  _wifiStatus = WiFi.begin(_ssid, _pass);
    
  String statusMessage = "Wifi Connecting Status: ";
  statusMessage.concat(_wifiStatus);
  _sendMessageCallback(statusMessage, Information);
}

void WebClientManager::printWifiStatus() 
{
	String wifiStatus = "SSID: ";
	wifiStatus.concat(WiFi.SSID());
	wifiStatus.concat("; Ip Address: ");
	wifiStatus.concat(WiFi.localIP());
	wifiStatus.concat("; Signal: ");
	wifiStatus.concat(WiFi.RSSI());
	wifiStatus.concat(" dBm");
	_sendMessageCallback(wifiStatus, Information);
}

int WebClientManager::getWiFiStatus()
{
	return _wifiStatus;
}

bool WebClientManager::isWifiConnected()
{
	return WiFi.isConnected();
}

int WebClientManager::socketConnectFailures()
{
	return _socketConnectFailures;
}

String WebClientManager::wifiStatus()
{
	String status = "Wifi Status: ";
	status.concat(_wifiStatus);
	status.concat("; socketConnected: ");
	status.concat(_socketConnected);
	status.concat("; currMillis: ");
	status.concat(millis());
	status.concat("; get requestSent: ");
	status.concat(_getRequestSent);
	status.concat("; get request timeout: ");
	status.concat(_getRequestTimeoutTime);
	status.concat("; post requestSent: ");
	status.concat(_postRequestSent);
	status.concat("; post request timeout: ");
	status.concat(_postRequestTimeoutTime);
	
	return status;
}


/// Get web request

bool WebClientManager::get(const unsigned long currMillis, const char *server, uint16_t port, const char *path)
{
	if (_postRequestSent)
		return false;
	
	connectToSocket(currMillis, server, port);
	
	if (!_getRequestSent && _connectResult > 0)
	{
		_socketConnected = true;
		_sendMessageCallback("connected to server", Information);
		// Make a HTTP request:
		_wifiClient->print("GET ");
		_wifiClient->print(path);
		_wifiClient->println(" HTTP/1.1");
		_wifiClient->print("Host: ");
		_wifiClient->println(server);
		_wifiClient->println("Connection: close");
		_wifiClient->println();
		_getRequestSent = true;
		_getRequestTimeoutTime = millis() + REQUEST_TIMEOUT_MS;
		return true;
	}

	return false;
}

bool WebClientManager::getRequestSent()
{
	if (_getRequestSent && millis() > _getRequestTimeoutTime)
	{
		_socketConnected = false;
		_getRequestSent = false;
		return false;
	}

	return _getRequestSent;
}

int WebClientManager::getReadResponse(char *buffer, int bufferSize)
{
	if (_getRequestSent && millis() > _getRequestTimeoutTime)
	{
		_socketConnected = false;
		_getRequestSent = false;
		return GET_REQUEST_TIMEOUT;
	}

	if (_postRequestSent)
		return POST_REQUEST_PENDING;
	
	if (!_getRequestSent || _wifiClient->peek() <= 0)
		return GET_REQUEST_PENDING;
	
	int count = 0;
	String response = "reading response; client available: ";
	response.concat(_wifiClient->available());
	_sendMessageCallback(response, Debug);
	
	while (_wifiClient->available() && count < bufferSize)
	{
		char c = _wifiClient->read();
		buffer[count] = c;
		count++;
	}  

	// if the server's disconnected, stop the client:
	_getRequestSent = false;
	
	if (!_wifiClient->connected())
	{
		_sendMessageCallback("disconnecting from server.", Information);
		_wifiClient->stop();
		_socketConnected = false;
	}
	
	return count;
}

bool WebClientManager::getHasResponse()
{
	return _getRequestSent && _wifiClient->peek() > 0;
}


/// Post web request

bool WebClientManager::post(const unsigned long currMillis, const char *server, uint16_t port, const char *path)
{
	if (_getRequestSent)
		return false;
	
	connectToSocket(currMillis, server, port);
	
	if (!_postRequestSent && _connectResult > 0)
	{
		_socketConnected = true;
		_sendMessageCallback("connected to server", Information);
		// Make a HTTP request:
		_wifiClient->print("POST ");
		_wifiClient->print(path);
		_wifiClient->println(" HTTP/1.1");
		_wifiClient->print("Host: ");
		_wifiClient->println(server);
		_wifiClient->println("Connection: close");
		_wifiClient->println();
		_postRequestSent = true;
		_postRequestTimeoutTime = millis() + REQUEST_TIMEOUT_MS;
		return true;
	}

	return false;
}

bool WebClientManager::postRequestSent()
{
	if (_postRequestSent && millis() > _postRequestTimeoutTime)
	{
		_socketConnected = false;
		_postRequestSent = false;
		return false;
	}

	return _postRequestSent;
}

int WebClientManager::postReadResponse(char *buffer, int bufferSize)
{
	if (_postRequestSent && millis() > _postRequestTimeoutTime)
	{
		_socketConnected = false;
		_postRequestSent = false;
		Serial.println("post timed out");
		return POST_REQUEST_TIEMOUT;
	}

	if (_getRequestSent)
	{
		Serial.println("get sent, unable to read post response");
		return GET_REQUEST_PENDING;
	}
	
	if (!_postRequestSent || _wifiClient->peek() <= 0)
	{
		Serial.println("No data back from post request");
		return POST_REQUEST_PENDING;
	}
	
	int count = 0;
	String response = "reading post response; client available: ";
	response.concat(_wifiClient->available());
	_sendMessageCallback(response, Debug);
	
	while (_wifiClient->available() && count < bufferSize)
	{
		char c = _wifiClient->read();
		buffer[count] = c;
		count++;
	}  

	// if the server's disconnected, stop the client:
	_postRequestSent = false;
	
	if (!_wifiClient->connected())
	{
		_sendMessageCallback("disconnecting from server.", Information);
		_wifiClient->stop();
		_socketConnected = false;
	}
	
	return count;
}

bool WebClientManager::postHasResponse()
{
	return _postRequestSent && _wifiClient->peek() > 0;
}


//// Json data

JsonResponse WebClientManager::htmlParseJsonBody(const String& data)
{
	JsonResponse result;
	result.success = false;

	int startPos = data.indexOf("\r\n\r\n");
	
	if (startPos == -1)
		return result;
	
	startPos = data.indexOf("\r\n", startPos + 4);
	
	int endPos = data.indexOf("\r\n0");
	
	if (endPos == -1)
		return result;
	
	String jsonResponse = data.substring(startPos + 2, endPos);
	
	JsonDocument doc;
	deserializeJson(doc, jsonResponse);
	
	
	result.success = doc["success"].as<bool>();
	result.json = doc["responseData"].as<String>();
	
	doc.clear();
	
	return result;
}

/// Private Methods

void WebClientManager::internalInitialize()
{
	_wifiStatus = WL_IDLE_STATUS;
	_wifiClient = new WiFiClient;
	_socketConnected = false;
	_getRequestSent = false;
	_postRequestSent = false;
	_socketConnectFailures = 0;
	
	WiFi.setTimeout(_wifiTimeout);
	connectToWiFi();
}

void WebClientManager::sendDebugStatus()
{
	_sendMessageCallback(wifiStatus(), Debug);
}

void WebClientManager::connectToSocket(const unsigned long currMillis, const char *server, uint16_t port)
{
	if (_wifiStatus == WL_CONNECTED && !_socketConnected && currMillis > _nextSocketConnectionRequest)
	{
		_sendMessageCallback("\nStarting connection to server", Debug);

		if (!_socketConnected)
		{
			// if you get a connection, report back via message callback :
			_connectResult = _wifiClient->connect(server, port);
			String connectResult = "client.connect result: ";
			connectResult.concat(_connectResult);
			_sendMessageCallback(connectResult, Information);
		}

		if (_connectResult > 0)
		{
			_socketConnected = true;
			_socketConnectFailures = 0;
		}
		else
		{
			_socketConnected = false;
			_getRequestSent = false;
			_sendMessageCallback("Socket not obtained", Error);
			_socketConnectFailures++;
			
			if (_socketConnectFailures >= MAXIMUM_FAILURES_TO_RECONNECT)
			{
				delete (_wifiClient);
				internalInitialize();
			}
		}
			
		_nextSocketConnectionRequest = currMillis + SOCKET_CONNECT_DELAY;
		sendDebugStatus();
	}
}