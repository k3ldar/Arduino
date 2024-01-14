// MasterSwapRoles

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define CE_PIN   9
#define CSN_PIN 10

const byte slaveAddress[5] = {'R','x','A','A','A'};
const byte masterAddress[5] = {'T','X','a','a','a'};


RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

char txNum = '0';

#define MAX_PACKET_DATA_SIZE 26
struct RFPacket
{
  unsigned char senderId;
  bool ackRequired;
  bool isAck;
  unsigned char packetId;
  short int messageId;
  char data[MAX_PACKET_DATA_SIZE];
};
RFPacket rfPacket;
RFPacket dataToReceive;

bool newData = false;

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 1000; // send once per second

//============

void setup() {
rfPacket.senderId = 215;
    Serial.begin(115200);

    Serial.println("MasterSwapRoles Starting");

    radio.begin();
    radio.setDataRate( RF24_250KBPS );

    radio.openWritingPipe(slaveAddress);
    radio.openReadingPipe(1, masterAddress);

    radio.setRetries(3,5); // delay, count
    send(); // to get things started
    prevMillis = millis(); // set clock
}

//=============

void loop() {
    currentMillis = millis();
    if (currentMillis - prevMillis >= txIntervalMillis) {
        send();
        prevMillis = millis();
    }
    getData();
    showData();
}

//====================

void send() {
RFPacket ts;
prepareData(&ts);
String s = "Some Random Data";
s.toCharArray(rfPacket.data, MAX_PACKET_DATA_SIZE);
copyData(&ts, &rfPacket);


        radio.stopListening();
            bool rslt;
            rslt = radio.write( &ts, sizeof(RFPacket) );
        radio.startListening();
        Serial.print("Data Sent ");
        Serial.print(rfPacket.data);
        if (rslt) {
            Serial.println("Acknowledge received");
            updateMessage();

    Serial.print("Data to Write; Sender: ");
    Serial.print(String(rfPacket.senderId));
    Serial.print("; PacketId: ");
    Serial.print(rfPacket.packetId);
    Serial.print("; MessageId: ");
    Serial.print(String(rfPacket.messageId));
    Serial.print("; Ack Required: ");
    Serial.print(rfPacket.ackRequired);
    Serial.print("; IsAck: ");
    Serial.print(rfPacket.isAck);
    Serial.print("; Data: ");
    Serial.println(rfPacket.data);



        }
        else {
            Serial.println("  Tx failed");
        }
}

//================

void getData() {

    if ( radio.available() ) {
        radio.read( &dataToReceive, sizeof(RFPacket) );
        newData = true;
    }
}

//================

void showData() {
    if (newData == true) {
        Serial.print("Data received ");
    Serial.print("Data received: ");
    Serial.print(String(dataToReceive.senderId));
    Serial.print("; PacketId: ");
    Serial.print(dataToReceive.packetId);
    Serial.print("; MessageId: ");
    Serial.print(String(dataToReceive.messageId));
    Serial.print("; Ack Required: ");
    Serial.print(dataToReceive.ackRequired);
    Serial.print("; IsAck: ");
    Serial.print(dataToReceive.isAck);
    Serial.print("; Data: ");
    Serial.println(dataToReceive.data);
        Serial.println();
        newData = false;
    }
}

//================

void updateMessage() {
        // so you can see that new data is being sent
    txNum += 1;
    if (txNum > '9') {
        txNum = '0';
    }
    char data[26] = "This is data\0";

    //dataToReceive.data = data;
    strncpy(dataToReceive.data, data, sizeof(data));
}

void copyData(RFPacket* to, const RFPacket* from)
{
  for (int i = 0; i < MAX_PACKET_DATA_SIZE; i++)
  {
    to->data[i] = from->data[i];
  }
}


void prepareData(RFPacket* packet)
{
  packet->senderId = 130;
packet->packetId = rfPacket.packetId;
packet->ackRequired = false;
packet->isAck = false;
packet->messageId = 235;
  for (int i = 0; i < MAX_PACKET_DATA_SIZE; i++)
  {
    packet->data[i] = '\0';
  }
}