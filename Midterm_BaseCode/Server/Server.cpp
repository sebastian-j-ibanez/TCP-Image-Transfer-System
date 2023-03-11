#include <windows.networking.sockets.h>
#include <iostream>
#include <string>
#include <fstream>
#include "../Client/PktDef.h"
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main()
{
	//starts Winsock DLLs		
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	//create server socket
	SOCKET ServerSocket;
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET) {
		WSACleanup();
		return 0;
	}

	//binds socket to address
	sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_addr.s_addr = INADDR_ANY;
	SvrAddr.sin_port = htons(27000);
	if (bind(ServerSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR)
	{
		closesocket(ServerSocket);
		WSACleanup();
		return 0;
	}

	//listen on a socket
	if (listen(ServerSocket, 1) == SOCKET_ERROR) {
		closesocket(ServerSocket);
		WSACleanup();
		return 0;
	}


	cout << "Waiting for client connection\n" << endl;

	//accepts a connection from a client
	SOCKET ConnectionSocket;
	ConnectionSocket = SOCKET_ERROR;
	if ((ConnectionSocket = accept(ServerSocket, NULL, NULL)) == SOCKET_ERROR) {
		closesocket(ServerSocket);
		WSACleanup();
		return 0;
	}

	cout << "Connection Established" << endl;

	char* RxBuffer = new char[PktDef::getMaxPacketSize()];
	
	bool receiveLoop = true;
	while (receiveLoop)
	{	
		recv(ConnectionSocket, RxBuffer, PktDef::getMaxPacketSize(), 0);

		PktDef currentPacket(RxBuffer);
		currentPacket.displayPacket();

		if (currentPacket.getFinFlag() == 0 && currentPacket.getBodyLength() != 0)		//Transmitted packet
		{
			char* fileBuffer = new char[MAX_BODY];
			memcpy(fileBuffer, currentPacket.getBodyAddress(), currentPacket.getBodyLength());

			ofstream outputStream;
			outputStream.open("image.jpeg", ios::app | ios::binary);
			if(outputStream.is_open()) outputStream.write(fileBuffer, MAX_BODY);
			outputStream.close();
		}
		else if (currentPacket.getFinFlag() == 1) receiveLoop = false;								//Transmission finished
		else
		{
			cout << "Error packet received" << endl;									//Transmission error
			receiveLoop = false;
		}

		//Send acknowledge packet
		PktDef ackPacket;
		ackPacket.setHeaderAckFlag(true);
		ackPacket.setHeaderSource(SERVER_ID);
		ackPacket.setHeaderDestination(CLIENT_ID);
		RxBuffer = ackPacket.serializePacket();
		send(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
	}
	
	closesocket(ConnectionSocket);	//closes incoming socket
	closesocket(ServerSocket);	    //closes server socket	
	WSACleanup();					//frees Winsock resources

	return 1;
}