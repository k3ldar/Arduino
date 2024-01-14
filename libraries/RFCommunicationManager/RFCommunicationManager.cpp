#include "RFCommunicationManager.h"

RFCommunicationManager::RFCommunicationManager(SendMessageCallback *sendMesageCallback, char senderId, bool isLocalHub, RF24 *radio)
{
  _sendMessageCallback = sendMesageCallback;
  _senderId = senderId;
  _radio = radio;
  _isLocalHub = isLocalHub;
  _isInitialized = false;
}

RFCommunicationManager::~RFCommunicationManager()
{
  
}

bool RFCommunicationManager::isInitialized()
{
  return _isInitialized;
}

void RFCommunicationManager::initialize()
{
  _nextHeartbeat = millis();

  _isInitialized = true;;
}

void RFCommunicationManager::process()
{
  if (_radio->available())
  {
	RFPacket rfData;
	cleanRFPacket(&rfData);
	_radio->read(&rfData, sizeof(RFPacket));
	
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
	_sendMessageCallback(debugMsg, Debug);

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
  
  unsigned long mil = millis();
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
	_radio->stopListening();

	bool result = _radio->write(rfPacket, sizeof(RFPacket));

	_radio->startListening();

	if (!result)
	{
		String errorMsg = "Failed to broadcast data; Sender: ";
		errorMsg.concat(rfPacket->senderId);
		errorMsg.concat("; IsChipConnected: ");
		errorMsg.concat(_radio->isChipConnected());
		errorMsg.concat("; Size: ");
		errorMsg.concat(sizeof(RFPacket));
		errorMsg.concat("; PacketId: ");
		errorMsg.concat(rfPacket->packetId);
		errorMsg.concat("; MessageId: ");
		errorMsg.concat(rfPacket->messageId);
		errorMsg.concat("; Ack Required: ");
		errorMsg.concat(rfPacket->ackRequired);
		errorMsg.concat("; IsAck: ");
		errorMsg.concat(rfPacket->isAck);
		errorMsg.concat("; Data: ");
		errorMsg.concat(rfPacket->data);
		_sendMessageCallback(errorMsg, Error);
	}

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