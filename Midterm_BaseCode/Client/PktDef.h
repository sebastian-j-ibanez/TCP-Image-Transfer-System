#pragma once
#include <iostream>
using namespace std;

const int MAX_BODY = 50;
const unsigned int CLIENT_ID = 15;
const unsigned int SERVER_ID = 25;

class PktDef
{
	struct Packet
	{
		struct PktHeader
		{
			unsigned char sourceID;
			unsigned char destinationID;
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
		Packet.Header.sourceID = ' ';
		Packet.Header.destinationID = ' ';
		Packet.Header.sequenceNum = 0;
		Packet.Header.finFlag = false;
		Packet.Header.ackFlag = false;
		Packet.Header.errFlag = false;
		Packet.Header.bodyLength = 0;

		Packet.bodyBuffer = nullptr;
		Packet.CRC = 0xF500FF75;

		pOutBuffer = nullptr;
	}

	PktDef(char* RxBuffer)
	{
		//Deserialize RxBuffer into PktDef
		size_t offset = 0;
		memcpy(&Packet.Header.sourceID, RxBuffer + offset, sizeof(Packet.Header.sourceID));
		offset += sizeof(Packet.Header.sourceID);
		memcpy(&Packet.Header.destinationID, RxBuffer + offset, sizeof(Packet.Header.destinationID));
		offset += sizeof(Packet.Header.destinationID);
		memcpy(&Packet.Header.sequenceNum, RxBuffer + offset, sizeof(Packet.Header.sequenceNum));
		offset += sizeof(Packet.Header.sequenceNum);
		memcpy(&Packet.Header.sequenceNum + 1, RxBuffer + offset, sizeof(char));
		offset += sizeof(char);
		memcpy(&Packet.Header.bodyLength, RxBuffer + offset, sizeof(Packet.Header.bodyLength));
		offset += sizeof(Packet.Header.bodyLength);
		
		//Allocate bodyBuffer memory based on bodyLength
		if (Packet.bodyBuffer) delete[] Packet.bodyBuffer;
		Packet.bodyBuffer = new char[Packet.Header.bodyLength];

		memcpy(Packet.bodyBuffer, RxBuffer + offset, Packet.Header.bodyLength);
		offset += Packet.Header.bodyLength;
		memcpy(&Packet.CRC, RxBuffer + offset, sizeof(Packet.CRC));
		offset += sizeof(Packet.CRC);

		pOutBuffer = nullptr;
	}

	char* serializePacket()
	{
		if (pOutBuffer) delete pOutBuffer;

		//Allocate buffer
		pOutBuffer = new char[getMaxPacketSize() + 500];

		//Serialize packet
		size_t offset = 0;
		memcpy(pOutBuffer + offset, &Packet.Header.sourceID, sizeof(Packet.Header.sourceID));
		offset += sizeof(Packet.Header.sourceID);
		memcpy(pOutBuffer + offset, &Packet.Header.destinationID, sizeof(Packet.Header.destinationID));
		offset += sizeof(Packet.Header.destinationID);
		memcpy(pOutBuffer + offset, &Packet.Header.sequenceNum, sizeof(Packet.Header.sequenceNum));
		offset += sizeof(Packet.Header.sequenceNum);

		//3 bit flags stored in 1 byte
		memcpy(pOutBuffer + offset, &Packet.Header.sequenceNum + 1, sizeof(char));
		offset += sizeof(char);

		memcpy(pOutBuffer + offset, &Packet.Header.bodyLength, sizeof(Packet.Header.bodyLength));
		offset += sizeof(Packet.Header.bodyLength);
		memcpy(pOutBuffer + offset, Packet.bodyBuffer, Packet.Header.bodyLength);
		offset += Packet.Header.bodyLength;
		memcpy(pOutBuffer + offset, &Packet.CRC, sizeof(Packet.CRC));
		offset += sizeof(Packet.CRC);

		return pOutBuffer;
	}

	~PktDef()
	{
		if(Packet.bodyBuffer) delete[] Packet.bodyBuffer;
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

	unsigned int getCRC()
	{
		return Packet.CRC;
	}

	size_t getPacketSize()
	{
		return sizeof(Packet.Header.sourceID) + sizeof(Packet.Header.destinationID) + sizeof(Packet.Header.sequenceNum) + sizeof(char) + sizeof(Packet.Header.bodyLength) + Packet.Header.bodyLength + sizeof(Packet.CRC);
	}

	static size_t getMaxPacketSize()
	{
		return sizeof(Packet.Header.sourceID) + sizeof(Packet.Header.destinationID) + sizeof(Packet.Header.sequenceNum) + sizeof(char) + sizeof(Packet.Header.bodyLength) + MAX_BODY + sizeof(Packet.CRC);
	}

	void swapSourceDestinationID()
	{
		unsigned char temp = Packet.Header.sourceID;
		Packet.Header.sourceID = Packet.Header.destinationID;
		Packet.Header.destinationID = temp;
	}

	void displayPacket()
	{
		cout << "Packet Information" << endl;
		cout << "Src: "<< (int)Packet.Header.sourceID << " Dest: " << (int)Packet.Header.destinationID << " SeqNum: " << Packet.Header.sequenceNum << " Flags (FIN/ACK/ERR): " << Packet.Header.finFlag << "/" << Packet.Header.ackFlag << "/" << Packet.Header.errFlag << "/" <<  endl;
	}

	void setBodyBuffer(char* inputBuffer, int size)
	{
		if (Packet.bodyBuffer) delete Packet.bodyBuffer;
		
		//Allocate bodyBuffer
		Packet.bodyBuffer = new char[size];

		//memcpy
		memcpy(Packet.bodyBuffer, inputBuffer, size);
	}

	const char* getBodyAddress()
	{
		return Packet.bodyBuffer;
	}
};
