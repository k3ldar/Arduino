#ifndef __RFCommunicationManager__
#define __RFCommunicationManager__

#include <SPI.h>             /* to handle the communication interface with the modem*/
#include <nRF24L01.h>        /* to handle this particular modem driver*/
#include <RF24.h>
#include "Common.h"

#define MAX_PACKET_DATA_SIZE 26


const short int HEARTBEAT_MESSAGE_ID = 1;

struct RFPacket
{
  unsigned char senderId;
  bool ackRequired;
  bool isAck;
  unsigned char packetId;
  short int messageId;
  char data[MAX_PACKET_DATA_SIZE];
};

class RFCommunicationManager
{
  private:
    RF24 *_radio;
    char _senderId;
    bool _isLocalHub;
    bool _isInitialized;
	unsigned long _nextHeartbeat;
	RFPacket _heartbeat;

	SendMessageCallback *_sendMessageCallback;
    bool broadcastRFData(RFPacket* rfPacket);
	void copyData(RFPacket* packetTo, const RFPacket* packetFrom);
	void copyData(RFPacket* packetTo, char data[MAX_PACKET_DATA_SIZE]);
	void prepareNewRFPacket(RFPacket* packet, const short int messageId);
	void cleanRFPacket(RFPacket* packet);
  public:
    RFCommunicationManager(char senderId, bool _isLocalHub, RF24 *radio);
    ~RFCommunicationManager();
    void initialize(SendMessageCallback *sendMesageCallback);
    void process();
    bool isChipConnected();
    bool isInitialized();
	bool sendMessage(bool ackRequired, short int messageId, char data[MAX_PACKET_DATA_SIZE]);
};

#endif