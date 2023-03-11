#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
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

	ifstream fileStream;
	fileStream.open("image.jpg");

	char* fileBuffer = new char[BUFFER_SIZE];
	char* TxBuffer = new char[BUFFER_SIZE];
	int packetNum = 0;

	if (fileStream.is_open())
	{
		while (!fileStream.eof())
		{
			//Read data into buffer
			fileStream.read(fileBuffer, BUFFER_SIZE);
			int size = 50;	// I DO NOT REMEMBER HOW TO SEEK TO END OF FILE IN ORDER TO GET SIZE. THIS CODE DOES NOT ACCOUNT FOR LAST PACKET, WHICH WILL BE UNDER 50 CHAR LONG. IDEALLY I CAN MODULUS THE ENTIRE SIZE OF FILE AND USE THAT TO CALCULATE HOW MANY PACKETS I WILL USE.

			//Initialize Packet
			PktDef currentPacket;

			//Set packet info
			currentPacket.setBodyBuffer(fileBuffer, size);
			currentPacket.setBodyLength(size);
			currentPacket.setHeaderSequenceNum(packetNum + 1);
			currentPacket.setHeaderSource(CLIENT_ID);
			currentPacket.setHeaderDestination(SERVER_ID);

			//Serialize packet
			TxBuffer = currentPacket.serializePacket();

			//Send packet
			send(ClientSocket, TxBuffer, sizeof(TxBuffer), 0);
			recv(ClientSocket, TxBuffer, sizeof(TxBuffer), 0);

			PktDef ackPacket(TxBuffer);
			ackPacket.displayPacket();

			fileStream.peek();
		}

		fileStream.close();
		PktDef finPacket;
		finPacket.setHeaderFinFlag(true);
		TxBuffer = finPacket.serializePacket();
		send(ClientSocket, TxBuffer, sizeof(TxBuffer), 0);
		recv(ClientSocket, TxBuffer, sizeof(TxBuffer), 0);

		PktDef ackPacket(TxBuffer);
		ackPacket.displayPacket();
	}
	else
	{
		PktDef errPacket;
		errPacket.setHeaderErrFlag(true);
		TxBuffer = errPacket.serializePacket();
		send(ClientSocket, TxBuffer, sizeof(TxBuffer), 0);
		recv(ClientSocket, TxBuffer, sizeof(TxBuffer), 0);

		PktDef ackPacket(TxBuffer);
		ackPacket.displayPacket();
	}
	
	//closes connection and socket
	closesocket(ClientSocket);

	//frees Winsock DLL resources
	WSACleanup();

	//This code has been added to simply keep the console window open until you
	//type a character.
	//int garbage;
	//cin >> garbage;

	return 1;
}