#pragma once
#include <iostream>
using namespace std;

const int BUFFER_SIZE = 50;
const unsigned int CLIENT_ID = 15;
const unsigned int SERVER_ID = 25;

class PktDef
{
	struct Packet
	{
		struct PktHeader
		{
			unsigned int sourceID : 4;
			unsigned int destinationID : 4;
			unsigned int sequenceNum;
			bool finFlag : 1;
			bool ackFlag : 1;
			bool errFlag : 1;
			unsigned int bodyLength;
		}Header;

		char* bodyBuffer;
		int CRC;

	}Packet;

	char* pOutBuffer;

public:
	PktDef()
	{
		Packet.Header.sourceID = NULL;
		Packet.Header.destinationID = NULL;
		Packet.Header.sequenceNum = 0;
		Packet.Header.finFlag = false;
		Packet.Header.ackFlag = false;
		Packet.Header.errFlag = false;
		Packet.Header.bodyLength = 0;

		Packet.bodyBuffer = new char[Packet.Header.bodyLength];
		Packet.CRC = 0xF500FF75;

		pOutBuffer = NULL;
	}

	PktDef(char* RxBuffer)
	{
		//Deserialize into PktDef
		size_t offset = 0;
		memcpy(&Packet.Header, RxBuffer + offset, sizeof(Packet.Header));
		offset += sizeof(Packet.Header);
		memcpy(&Packet.bodyBuffer, RxBuffer + offset, Packet.Header.bodyLength);
		offset += sizeof(Packet.Header.bodyLength);
		memcpy(&Packet.CRC, RxBuffer + offset, sizeof(Packet.CRC));
		offset += sizeof(Packet.CRC);

		pOutBuffer = NULL;
	}

	~PktDef()
	{
		delete Packet.bodyBuffer;
		delete pOutBuffer;
	}

	void setHeaderSource(unsigned int input)
	{
		Packet.Header.sourceID = input;
	}

	void setHeaderDestination(unsigned int input)
	{
		Packet.Header.destinationID = input;
	}

	void setHeaderSequenceNum(unsigned int input)
	{
		Packet.Header.sequenceNum = input;
	}

	void setHeaderAckFlag(bool input)
	{
		Packet.Header.ackFlag = input;
	}

	void setHeaderFinFlag(bool input)
	{
		Packet.Header.finFlag = input;
	}

	void setHeaderErrFlag(bool input)
	{
		Packet.Header.errFlag = input;
	}

	void setBodyLength(unsigned int input)
	{
		Packet.Header.bodyLength = input;
	}

	int getBodyLength()
	{
		return Packet.Header.bodyLength;
	}

	bool getFinFlag()
	{
		return Packet.Header.finFlag;
	}

	bool getErrFlag()
	{
		return Packet.Header.errFlag;
	}

	unsigned int getCRC()
	{
		return Packet.CRC;
	}

	size_t getPacketSize()
	{
		return sizeof(Packet.Header) + Packet.Header.bodyLength + sizeof(Packet.CRC);
	}

	void swapSourceDestinationID()
	{
		unsigned int temp = Packet.Header.sourceID;
		Packet.Header.sourceID = Packet.Header.destinationID;
		Packet.Header.destinationID = temp;
	}

	void displayPacket()
	{
		cout << "Packet Information" << endl;
		cout << "Src: "<< Packet.CRC << "Dest: " << Packet.Header.destinationID << "SeqNum: " << Packet.Header.sequenceNum << "Flages (FIN/ACK/ERR): " << Packet.Header.finFlag << "/" << Packet.Header.ackFlag << "/" << Packet.Header.errFlag << "/" <<  endl;
	}

	void setBodyBuffer(char* inputBuffer, int size)
	{
		if (Packet.bodyBuffer)
			delete Packet.bodyBuffer;
		
		//Allocate bodyBuffer
		Packet.bodyBuffer = new char[size];

		//memcpy
		memcpy(Packet.bodyBuffer, inputBuffer, size);
	}

	const char* getBodyAddress()
	{
		return Packet.bodyBuffer;
	}

	char* serializePacket()
	{
		if (pOutBuffer)
			delete pOutBuffer;

		size_t PacketSize = getPacketSize();

		//Allocate buffer
		pOutBuffer = new char[PacketSize];

		//Serialize packet
		size_t offset = 0;
		memcpy(pOutBuffer, &Packet.Header, sizeof(Packet.Header));
		offset += sizeof(Packet.Header);
		memcpy(pOutBuffer + offset, &Packet.bodyBuffer, Packet.Header.bodyLength);
		offset += Packet.Header.bodyLength;
		memcpy(pOutBuffer + offset, &Packet.CRC, sizeof(Packet.CRC));
		offset += sizeof(Packet.CRC);

		return pOutBuffer;
	}


};
