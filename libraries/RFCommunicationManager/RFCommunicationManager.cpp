#include "RFCommunicationManager.h"

RFCommunicationManager::RFCommunicationManager(SendMessageCallback* sendMesageCallback, char senderId, bool isLocalHub, RF24 *radio)
{
	_sendMessageCallback = sendMesageCallback;
	_senderId = senderId;
	_radio = radio;
	_isLocalHub = isLocalHub;
	_nextAutoReconnect = millis();
	_nextHeartbeat = _nextAutoReconnect;
}

RFCommunicationManager::~RFCommunicationManager()
{
  
}

void RFCommunicationManager::process()
{
	unsigned long mil = millis();
	_isConnected = _radio->isChipConnected();

	if (_isConnected && _radio->available())
	{
		RFPacket rfData;
		cleanRFPacket(&rfData);
		_radio->read(&rfData, sizeof(RFPacket));
		
		if (rfData.senderId > 0)
		{
			String debugMsg = "Message Received; Sender: ";
			debugMsg.concat(String(rfData.senderId));
			debugMsg.concat("; PacketId: ");
			debugMsg.concat(String(rfData.packetId));
			debugMsg.concat("; MessageId: ");
			debugMsg.concat(String(rfData.messageId));
			debugMsg.concat("; Ack Required: ");
			debugMsg.concat(String(rfData.ackRequired));
			debugMsg.concat("; IsAck: ");
			debugMsg.concat(rfData.isAck);
			debugMsg.concat("; Data: ");
			debugMsg.concat(rfData.data);
			
			if (rfData.isAck)
				_sendMessageCallback(debugMsg, Debug);
			else
				_sendMessageCallback(debugMsg, Information);

			if (_isLocalHub)
			{
				broadcastRFData(&rfData);
			}
			else if (rfData.ackRequired && !rfData.isAck && rfData.senderId != _senderId)
			{
				RFPacket ackData;
				prepareNewRFPacket(&ackData, rfData.messageId);
				ackData.isAck = true;
				copyData(&ackData, &rfData);
				broadcastRFData(&ackData);
				_sendMessageCallback("Ack Sent", Debug);
			}
		}
	}
	else if (!_isConnected && mil > _nextAutoReconnect)
	{
		_nextAutoReconnect = mil + 2000;
	}
	
	sendHeartbeat(mil);
}

void RFCommunicationManager::sendHeartbeat(unsigned long mil)
{
	if (mil > _nextHeartbeat)
	{
		_nextHeartbeat = mil + 2000;
		char message[MAX_PACKET_DATA_SIZE] = "heartbeat";
		bool result = sendMessage(true, HEARTBEAT_MESSAGE_ID, message);

		if (!result)
		{
			_sendMessageCallback("Failed to send heartbeat", Error);
		}
		else
		{
			_sendMessageCallback("heartbeat sent", Debug);
		}
	}
}

bool RFCommunicationManager::broadcastRFData(RFPacket* rfPacket)
{
	bool result = _radio->isChipConnected();
	
	if (result)
	{
		_radio->stopListening();

		result = _radio->write(rfPacket, sizeof(RFPacket), false);

		_radio->startListening();
	}
	
	String msg = "";
	
	if (result)
		msg = "Message Broadcast; Sender: ";
	else
		msg = "Failed to broadcast data; Sender: ";
		 
	msg.concat(rfPacket->senderId);
	msg.concat("; IsChipConnected: ");
	msg.concat(_radio->isChipConnected());
	msg.concat("; Size: ");
	msg.concat(sizeof(RFPacket));
	msg.concat("; PacketId: ");
	msg.concat(rfPacket->packetId);
	msg.concat("; MessageId: ");
	msg.concat(rfPacket->messageId);
	msg.concat("; Ack Required: ");
	msg.concat(rfPacket->ackRequired);
	msg.concat("; IsAck: ");
	msg.concat(rfPacket->isAck);
	msg.concat("; Data: ");
	msg.concat(rfPacket->data);
		
	if (result)
		_sendMessageCallback(msg, Debug);
	else
		_sendMessageCallback(msg, Error);

	return result;
}

bool RFCommunicationManager::sendMessage(bool ackRequired, short int messageId, char data[MAX_PACKET_DATA_SIZE])
{
	RFPacket rfData;
	prepareNewRFPacket(&rfData, messageId);
	rfData.ackRequired = ackRequired;
	copyData(&rfData, data);

	bool result = broadcastRFData(&rfData);

	return result;
}

bool RFCommunicationManager::connectToRadio(const byte* readAddress, const byte* writeAddress)
{
	bool isConnected = _radio->begin();
	
	if (!isConnected)
	{
		Serial.println("Radio not responding");
		return false;
	}  

	_radio->openWritingPipe(writeAddress);

	_radio->openReadingPipe(1, readAddress);

	_radio->startListening();

	Serial.print("Data rate: ");
	Serial.println(_radio->getDataRate());
	Serial.print("Ip Variant: ");
	Serial.println(_radio->isPVariant());
	Serial.print("Is Connected: ");
	Serial.println(_radio->isChipConnected() ? "Yes" : "No");
	
	_radio->setPALevel(RF24_PA_MAX);
	_radio->setDataRate(RF24_1MBPS);
	_radio->setRetries(0, 0);
	_radio->setAutoAck(false);
	_isConnected = true;
	
	return _isConnected;
}

bool RFCommunicationManager::canReconnect()
{
	return !_isConnected && millis() > _nextAutoReconnect;
}

void RFCommunicationManager::copyData(RFPacket* packetTo, char data[MAX_PACKET_DATA_SIZE])
{
	for (int i = 0; i < MAX_PACKET_DATA_SIZE; i++)
	{
		packetTo->data[i] = data[i];
	}
}

void RFCommunicationManager::copyData(RFPacket* packetTo, const RFPacket* packetFrom)
{
	for (int i = 0; i < MAX_PACKET_DATA_SIZE; i++)
	{
		packetTo->data[i] = packetFrom->data[i];
	}
}

void RFCommunicationManager::cleanRFPacket(RFPacket* packet)
{
	packet->senderId = 0;
	packet->packetId = 0;
	packet->ackRequired = false;
	packet->isAck = false;
	packet->messageId = 0;

	for (int i = 0; i < MAX_PACKET_DATA_SIZE; i++)
	{
		packet->data[i] = '\0';
	}	
}

void RFCommunicationManager::prepareNewRFPacket(RFPacket* packet, const short int messageId)
{
	packet->senderId = _senderId;
	packet->packetId = 0;
	packet->ackRequired = false;
	packet->isAck = false;
	packet->messageId = messageId;

	for (int i = 0; i < MAX_PACKET_DATA_SIZE; i++)
	{
		packet->data[i] = '\0';
	}
}