
#include "Common.h"
#include "WebClientManager.h"
#include "arduino_secrets.h" 
#include "SerialCommandManager.h"
#include <ArduinoJson.h>

#include "MemoryFree.h"

#define UPDATE_SERVER_MILLISECONDS 60000
#define TEMPERATURE_RETRIEVE_MILLISECONDS 60000

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
//IPAddress server(127, 0, 0, 1);
char server[] = "192.168.8.201";//"127.0.0.1";//"192.168.8.200";//"www.google.com";    // name address for Google (using DNS)
uint16_t port = 7100;

long DeviceId = -1;

WebClientManager webClient;

SerialCommandManager commandMgr(&CommandReceived, '\n', ':', '=', 500, 256);
unsigned long _nextSendUpdate = 0;
unsigned long _nextGet = 0;


void SendMessage(String message, MessageType messageType)
{
  switch (messageType)
  {
    case Debug:
      commandMgr.sendDebug(message, "Wi");
      return;
    case Error:
      commandMgr.sendError(message, "Wi");
      return;
    default:
      commandMgr.sendCommand("Wi", message);
      return;
  }
}

void CommandReceived()
{
  if (commandMgr.getCommand() == "WIFI")
  {
    commandMgr.sendCommand("WIFI", webClient.wifiStatus());
    return;
  }

  if (commandMgr.getCommand() == "DID")
  {
    commandMgr.sendCommand("DID", String(DeviceId));
    return;
  }
  
  if (commandMgr.getCommand() == "FREG")
  {
    registerDevice(millis());
    return;
  }
}

void setup()
{
    Serial.begin(230400);
    while (!Serial);

    webClient.initialize(&SendMessage, 10000, ssid, pass);
}

void loop()
{
    commandMgr.readCommands();

    unsigned long currMillis = millis();

    process(currMillis);

    webClient.process();

    if (DeviceId == -1)
    {
        registerDevice(currMillis);
    }
    else
    {
        getTemperature(currMillis);
    }

    processFailures();

    if (!webClient.getRequestSent() && !webClient.postRequestSent())
        delay(200);
}

void process(unsigned long currMillis)
{
    if (currMillis > _nextSendUpdate)
    {
        updateServer(currMillis);          
    }
}

void registerDevice(unsigned long currMillis)
{
    if (DeviceId > -1)
        return;

    if (webClient.isWifiConnected())
    {
        if (webClient.getRequestSent() && webClient.getHasResponse())
        {
            int bufferSize = 500;
            char buffer[bufferSize];
            int bytesRead = webClient.getReadResponse(buffer, bufferSize);

            Serial.print("Data Length: ");
            Serial.println(bytesRead);

            if (bytesRead > 0)
            {
                Serial.println(String(buffer, bytesRead));

                JsonResponse response = webClient.htmlParseJsonBody(String(buffer));
                
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
        else if (!webClient.getRequestSent())
        {
            char tempPath[30];
            sprintf(tempPath, "/Device/RegisterDevice/%d/", WeatherStation);
            webClient.get(currMillis, server, port, tempPath);
        }
    }
}

void getTemperature(unsigned long currMillis)
{
    if (currMillis > _nextGet && webClient.isWifiConnected())
    {
        if (webClient.getRequestSent() && webClient.getHasResponse())
        {
            int bufferSize = 4000;
            char buffer[bufferSize];
            int bytesRead = 0;
            
            bytesRead = webClient.getReadResponse(buffer, bufferSize);
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
                    _nextGet = currMillis + TEMPERATURE_RETRIEVE_MILLISECONDS;

            }
        }
        else if (!webClient.getRequestSent())
        {
            char tempPath[50];
            sprintf(tempPath, "/Device/GetCurrentTemperature/%ld/", DeviceId);
            webClient.get(currMillis, server, port, tempPath);
        }
    }
}

void processFailures()
{
    // not required but reports failure
    int failures = webClient.socketConnectFailures();

    if (failures > 0)
    {
        String failureCount = "Failure Count: ";
        failureCount.concat(failures);
        SendMessage(failureCount, Information);
    }
}

void updateServer(unsigned long currMillis)
{
    //SendMessage("Update Server", Information);

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
            float temp = 8.4;
            float humid = 45.6;
            bool rain = false;
            char tempPath[80];
            sprintf(tempPath, "/Device/UpdateWeather/%ld/%f/%f/%d/", DeviceId, temp, humid, rain);
            webClient.post(currMillis, server, port, tempPath);
        }
    }
    else
    {
        Serial.println("Wifi not connected");
    }
}


