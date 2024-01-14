#ifndef __Common__
#define __Common__

enum MessageType
{
	Error,
	Debug,
	Information
};

typedef void SendMessageCallback(String message, MessageType messageType);

#endif