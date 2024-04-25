#include "WebClientManager.h"

WebClientManager::WebClientManager()
{
	_connectResult = -1;
	_nextWiFiConnect = 0;
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

void WebClientManager::process(unsigned long currMillis)
{
	if (isWaiting(currMillis))
		return;
	
	switch (_wifiStatus)
	{
		case WL_CONNECTED:
			if (_wifiConnectFailures > 0)
				_wifiConnectFailures = 0;
			
			break;
			
		case WL_CONNECTING:
			_wifiStatus = getWiFiStatus();
			printWifiStatus();
			
			break;
		
		case WL_CONNECTION_LOST:
		case WL_CONNECT_FAILED:
			Serial.println("Connection Failed/Lost");

			if (_socketConnected)
				disconnectFromSocket();
			
			_wifiStatus = WL_DISCONNECTING;
			wait(currMillis, DISCONNECT_TIMEOUT);
			_wifiConnectFailures++;
			
			break;
			
		case WL_DISCONNECTING:
			printWifiStatus();
			_wifiStatus = WiFi.isConnected();
			
			break;
			
		case WL_NO_MODULE:
			_sendMessageCallback("Communication with WiFi module failed!", Information);
			
			break;

	}
}

bool WebClientManager::isWaiting(unsigned long currMillis)
{
	return _nextWiFiConnect > currMillis;
}

void WebClientManager::connectToWiFi()
{
  // attempt to connect to WiFi network:
  String connectMessage = "Attempting to connect to SSID: ";
  connectMessage.concat(_ssid);
  _sendMessageCallback(connectMessage, Information);
  _wifiConnectFailures = 0;
  
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  _wifiStatus = WiFi.begin(_ssid, _pass);
  wait(millis(), CONNECT_TIMEOUT);
  String statusMessage = "Wifi Connecting Status: ";
  statusMessage.concat(_wifiStatus);
  _sendMessageCallback(statusMessage, Information);
}

void WebClientManager::printWifiStatus() 
{
	_sendMessageCallback(wifiStatus(), Information);
}

int WebClientManager::getWiFiStatus()
{
	unsigned long currMillis = millis();
	
	if (currMillis < _nextStatus)
		return _wifiStatus;
	
	_wifiStatus = WiFi.isConnected();
	_nextStatus = currMillis + MIN_STATUS_TIME;
}

bool WebClientManager::isWifiConnected()
{
	return getWiFiStatus();
}

int WebClientManager::wifiConnectFailures()
{
	return _wifiConnectFailures;
}

int WebClientManager::socketConnectFailures()
{
	return _socketConnectFailures;
}

String WebClientManager::wifiStatus()
{
	String status = "SSID: ";
	status.concat(WiFi.SSID());
	status.concat("; RSSI: ");
	status.concat(WiFi.RSSI());
	status.concat(" dBm; Ip Address: ");
	status.concat(WiFi.localIP());
	status.concat("; Wifi Status: ");
	status.concat(wifiConnectionStatus());
	status.concat("; socketConnected: ");
	status.concat(_socketConnected);
	status.concat("; currMillis: ");
	status.concat(millis());
	status.concat("; Get Sent: ");
	status.concat(_getRequestSent);
	status.concat("; Get Timeout: ");
	status.concat(_getRequestTimeoutTime);
	status.concat("; Post Sent: ");
	status.concat(_postRequestSent);
	status.concat("; Post Timeout: ");
	status.concat(_postRequestTimeoutTime);
	status.concat("; next Socket: ");
	status.concat(_nextSocketConnectionRequest);
	
	return status;
}

String WebClientManager::wifiConnectionStatus()
{
	switch (_wifiStatus)
	{
		case WL_IDLE_STATUS:
			return "Idle";
		case WL_NO_SSID_AVAIL:
			return "No SSID";
		case WL_SCAN_COMPLETED:
			return "Scan Completed";
		case WL_CONNECTED:
			return "Connected";
		case WL_CONNECT_FAILED:
			return "Connect Failed";
		case WL_CONNECTION_LOST:
			return "Connection Lost";
		case WL_DISCONNECTED:
			return "Disconnected";
		case WL_AP_LISTENING:
			return "AP Listening";
		case WL_AP_CONNECTED:
			return "AP Connected";
		case WL_AP_FAILED:
			return "AP Failed";
		case WL_CONNECTING:
			return "Connecting";
		case WL_DISCONNECTING:
			return "Disconnecting";
	}
	
	return "Unknown";
}

void WebClientManager::setTimeout(int timeout)
{
	_wifiClient->setConnectionTimeout(timeout);
}

long WebClientManager::getRssi()
{
	unsigned long currMillis = millis();
	
	if (currMillis > _nextRssi)
	{	
		_lastRssi = WiFi.RSSI();
		_nextRssi = currMillis + MIN_STATUS_TIME;
	}
	
	return _lastRssi;
}

bool WebClientManager::canConnectToSocket(unsigned long currMillis)
{
	return currMillis > _nextSocketConnectionRequest;
}


/// Get web request

bool WebClientManager::get(const unsigned long currMillis, const char *server, uint16_t port, const char *path)
{
	if (_postRequestSent)
		return false;
	
	if (!connectToSocket(currMillis, server, port))
		return false;
	
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
		//_wifiClient->println("Connection: close");
		_wifiClient->println();
		_getRequestSent = true;
		_getRequestTimeoutTime = currMillis + REQUEST_TIMEOUT_MS;
		return true;
	}

	_nextSocketConnectionRequest = currMillis + SOCKET_CONNECT_DELAY;
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
	
	if (_wifiClient->connected())
	{
		_sendMessageCallback("disconnecting from server.", Information);
		_wifiClient->flush();
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
	
	if (!connectToSocket(currMillis, server, port))
		return false;
	
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
		_wifiClient->println("Content-Length: 0");
		//_wifiClient->println("Connection: close");
		_wifiClient->println();
		_postRequestSent = true;
		_postRequestTimeoutTime = currMillis + REQUEST_TIMEOUT_MS;
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
	
	if (_wifiClient->connected())
	{
		_sendMessageCallback("disconnecting from server.", Information);
		_wifiClient->flush();
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

!!!!!!!!   would be nice to internal initialize after n connection errors

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

bool WebClientManager::connectToSocket(const unsigned long currMillis, const char *server, uint16_t port)
{
	if (_wifiStatus == WL_CONNECTED && !_socketConnected && currMillis > _nextSocketConnectionRequest)
	{
		_sendMessageCallback("\nStarting connection to server", Debug);

		if (!_socketConnected)
		{
			int timeout = _wifiClient->getConnectionTimeout();
			_wifiClient->setConnectionTimeout(1000);
			// if you get a connection, report back via message callback :
			_connectResult = _wifiClient->connect(server, port);
			_wifiClient->setConnectionTimeout(timeout);
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
			_nextSocketConnectionRequest = currMillis + SOCKET_CONNECT_DELAY;
			_socketConnected = false;
			_getRequestSent = false;
			_sendMessageCallback("Socket not obtained", Error);
			_socketConnectFailures++;
			
			if (_socketConnectFailures > MAX_CONNECT_FAILURES)
			{
				_sendMessageCallback(wifiStatus(), Information);
				_socketConnectFailures = 0;
				disconnectFromSocket();
			}
		}
			
		sendDebugStatus();
		return true;
	}

	return false;
}

void WebClientManager::disconnectFromSocket()
{
	if (!_socketConnected)
		return;
	
	_wifiClient->stop();
	_socketConnected = false;
	wait(millis(), DISCONNECT_TIMEOUT);
}

void WebClientManager::wait(unsigned long currMillis, unsigned long waitTime)
{
	_nextWiFiConnect = currMillis + waitTime;
}