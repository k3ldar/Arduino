#include "OpOverkill.h"

OpOverkill::OpOverkill()
{
	_deviceId = DEVICE_NOT_FOUND;
}

OpOverkill::~OpOverkill()
{
}

void OpOverkill::initialize(SendMessageCallback *sendMesageCallback, WebClientManager *webClient)
{
	_sendMessageCallback = sendMesageCallback;
	_webClient = webClient;
}

void OpOverkill::process(unsigned long millis)
{
	if (_deviceId == -1)
    {
        registerDevice(currMillis);
    }

}

long OpOverkill::getDeviceId()
{
	return _deviceId;
}


void OpOverkill::processFailures()
{
    // not required but reports failure
    int failures = webClient->socketConnectFailures();

    if (failures > 0)
    {
        String failureCount = "Failure Count: ";
        failureCount.concat(failures);
        _sendMessageCallback(failureCount, Information);
    }
}


void OpOverkill::updateServer(float)
{
    if (webClient.isWifiConnected())
    {
        if (webClient.postRequestSent() && webClient.postHasResponse())
        {
            int bufferSize = 4000;
            char buffer[bufferSize];
            int bytesRead = 0;
            
            bytesRead = webClient.postReadResponse(buffer, bufferSize);
            Serial.print("Data Length: ");
            Serial.println(bytesRead);

            if (bytesRead > 0)
            {
                Serial.println(String(buffer, bytesRead));

                String data = String(buffer);

                JsonResponse response = webClient.htmlParseJsonBody(data);
                
                Serial.print("Json Success: ");
                Serial.println(response.success);
                Serial.print("Response Data: ");
                Serial.println(response.json);

                if (response.success)
                    _nextSendUpdate = currMillis + UPDATE_SERVER_MILLISECONDS;
            }
        }
        else if (!webClient.postRequestSent())
        {
            Serial.println("Sending post request");
            float temp = weatherStation.getTemperature();
            float humid = weatherStation.getHumidity();
            float rainSensor = weatherStation.getRainSensor();
            bool rain = false;
            char tempPath[80];
            sprintf(tempPath, "/Device/UpdateWeather/%ld/%f/%f/%f/%d/", DeviceId, temp, humid, rainSensor, rain);
            webClient.post(currMillis, server, port, tempPath);
        }
    }
    else
    {
        Serial.println("Wifi not connected");
    }
}

// private members


void OpOverkill::registerDevice(unsigned long currMillis)
{
    if (_deviceId > -1)
        return;

    if (_webClient->isWifiConnected())
    {
        if (_webClient.getRequestSent() && _webClient.getHasResponse())
        {
            int bufferSize = 500;
            char buffer[bufferSize];
            int bytesRead = _webClient.getReadResponse(buffer, bufferSize);

            Serial.print("Data Length: ");
            Serial.println(bytesRead);

            if (bytesRead > 0)
            {
                Serial.println(String(buffer, bytesRead));

                JsonResponse response = _webClient.htmlParseJsonBody(String(buffer));
                
                Serial.print("Json Success: ");
                Serial.println(response.success);
                Serial.print("Response Data: ");
                Serial.println(response.json);

                if (response.success)
                {
                    DeviceId = response.json.toInt();
                }
                else
                {
                    Serial.println("Failed to register device");
                }

                Serial.print("Device Id: ");
                Serial.println(DeviceId);
            }
        }
        else if (!_webClient.getRequestSent())
        {
            char tempPath[30];
            sprintf(tempPath, "/Device/RegisterDevice/%d/", DeviceWeatherStation);
            _webClient.get(currMillis, server, port, tempPath);
        }
    }
}