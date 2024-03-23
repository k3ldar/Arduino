
#include "Common.h"
#include "WebClientManager.h"
#include "arduino_secrets.h" 
#include "SerialCommandManager.h"
#include <ArduinoJson.h>

#include "MemoryFree.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
//IPAddress server(127, 0, 0, 1);
char server[] = "192.168.8.201";//"127.0.0.1";//"192.168.8.200";//"www.google.com";    // name address for Google (using DNS)
uint16_t port = 7100;

WebClientManager webClient;

SerialCommandManager commandMgr(&CommandReceived, '\n', ':', '=', 500, 256);
unsigned long _nextMsg = 0;
unsigned long _nextGet = 0;


void SendMessage(String message, MessageType messageType)
{
  switch (messageType)
  {
    case Debug:
      commandMgr.sendDebug(message, "Wi");
      Serial.print("Next Get: ");
      Serial.println(_nextGet);
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
}

void setup()
{
    Serial.begin(230400);
    while (!Serial);

    webClient.initialize(&SendMessage, 10000, ssid, pass);
    //modem.debug(Serial, 0);
}

void loop()
{
    commandMgr.readCommands();

    unsigned long currMillis = millis();

    if (currMillis > _nextMsg)
    {
        SendMessage("Other Processing", Information);
        _nextMsg = currMillis + 1000;
    }

    webClient.process();

    if (currMillis > _nextGet && webClient.isWifiConnected())
    {
        if (webClient.getRequestSent() && webClient.getHasResponse())
        {
            int bufferSize = 4000;
            char buffer[bufferSize];
            _nextGet = currMillis + 7000;
            int bytesRead = 0;
            
            bytesRead = webClient.getReadResponse(buffer, bufferSize);
            Serial.print("Data Length: ");
            Serial.println(bytesRead);

            if (bytesRead > 0)
            {
              Serial.print("Mem Start: ");
              Serial.println(freeMemory());
                Serial.println(String(buffer, bytesRead));

                String data = String(buffer);

                JsonResponse response = webClient.htmlParseJsonBody(data);
                
                Serial.print("Json Success: ");
                Serial.println(response.success);
                Serial.print("Response Data: ");
                Serial.println(response.json);
              Serial.print("Mem End: ");
              Serial.println(freeMemory());
            }

        }
        else if (!webClient.getRequestSent())
        {
            webClient.get(currMillis, server, port, "/Device/GetCurrentTemperature/345/");
        }
    }


    // not required but reports failure
    int failures = webClient.socketConnectFailures();

    if (failures > 0)
    {
        String failureCount = "Failure Count: ";
        failureCount.concat(failures);
        SendMessage(failureCount, Information);
    }
}


