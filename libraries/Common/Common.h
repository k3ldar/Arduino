#ifndef __Common__H
#define __Common__H


enum MessageType
{
	Error,
	Debug,
	Information
};

typedef void SendMessageCallback(String message, MessageType messageType);


struct JsonResponse
{
	bool success;
	String json;
};
#endif