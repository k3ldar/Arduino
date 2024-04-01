#ifndef __OpOverkill__H
#define __OpOverkill__H

#include "WebClientManager.h"
#include "WebClientManager.h"

#define DEVICE_NOT_FOUND -1;

class OpOverkill
{
private:
	SendMessageCallback *_sendMessageCallback;
	WebClientManager *_webClient;
	long _deviceId;

	void registerDevice(unsigned long currMillis);
public:
	OpOverkill();
	~OpOverkill();
	void initialize(SendMessageCallback *sendMesageCallback, WebClientManager *webClient); 
	void process(unsigned long millis);
	
	long getDeviceId();
	
	void processFailures();
};

#endif