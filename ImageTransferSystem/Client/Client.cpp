#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <math.h>
#include "PktDef.h"
using namespace std;

int main()
{
	//starts Winsock DLLs
	WSADATA wsaData;
	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		return 0;
	}

	//initializes socket. SOCK_STREAM: TCP
	SOCKET ClientSocket;
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == INVALID_SOCKET) {
		WSACleanup();
		return 0;
	}

	//Connect socket to specified server
	sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET;						//Address family type itnernet
	SvrAddr.sin_port = htons(27000);					//port (host to network conversion)
	SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");	//IP address
	if ((connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
		closesocket(ClientSocket);
		WSACleanup();
		return 0;
	}

	char* TxBuffer = new char[PktDef::getMaxPacketSize()];
	char* fileBuffer = new char[MAX_BODY];

	ifstream fileStream;
	fileStream.open("image.jpeg", std::ios::binary);

	if (fileStream.is_open())
	{
		//Get file size
		fileStream.seekg(0, fileStream.end);
		int size = fileStream.tellg();
		fileStream.seekg(0, fileStream.beg);

		double totalPackets = size % MAX_BODY;
		
		int sequenceNum = 0;

		while(fileStream.good() && !fileStream.eof())
		{
			//Initialize Packet
			PktDef currentPacket;

			fileStream.read(fileBuffer, MAX_BODY);

			//Set packet info
			currentPacket.setHeaderSource(CLIENT_ID);
			currentPacket.setHeaderDestination(SERVER_ID);
			currentPacket.setHeaderSequenceNum(sequenceNum++);
			currentPacket.setBodyLength(MAX_BODY);
			currentPacket.setBodyBuffer(fileBuffer, MAX_BODY);

			//Serialize packet
			TxBuffer = currentPacket.serializePacket();

			//Send packet
			send(ClientSocket, TxBuffer, PktDef::getMaxPacketSize(), 0);
			
			recv(ClientSocket, TxBuffer, PktDef::getMaxPacketSize(), 0);	//Receive response
			PktDef ackPacket(TxBuffer);							//Create response packet
			ackPacket.displayPacket();							//Display response
				
			fileStream.peek();									//Peek file to avoid reading past eof
		}

		fileStream.close();

		//Send Fin Packet
		PktDef finPacket;
		finPacket.setHeaderFinFlag(true);
		finPacket.setHeaderSource(CLIENT_ID);
		finPacket.setHeaderDestination(SERVER_ID);
		TxBuffer = finPacket.serializePacket();
		send(ClientSocket, TxBuffer, PktDef::getMaxPacketSize(), 0);
		
		recv(ClientSocket, TxBuffer, PktDef::getMaxPacketSize(), 0);		//Receive response
		PktDef ackPacket(TxBuffer);								//Create response packet
		ackPacket.displayPacket();								//Display response
	}
	else
	{
		//Send Error Packet
		PktDef errPacket;
		errPacket.setHeaderErrFlag(true);
		errPacket.setHeaderSource(CLIENT_ID);
		errPacket.setHeaderDestination(SERVER_ID);
		TxBuffer = errPacket.serializePacket();
		send(ClientSocket, TxBuffer, PktDef::getMaxPacketSize(), 0);
		
		recv(ClientSocket, TxBuffer, PktDef::getMaxPacketSize(), 0);		//Receive response
		PktDef ackPacket(TxBuffer);								//Create response packet
		ackPacket.displayPacket();								//Display response
	}
	
	//closes connection and socket
	closesocket(ClientSocket);

	//frees Winsock DLL resources
	WSACleanup();

	string wait;
	cin >> wait;

	return 1;
}