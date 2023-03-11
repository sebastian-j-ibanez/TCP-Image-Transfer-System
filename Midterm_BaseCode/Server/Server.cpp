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

	char* RxBuffer = new char[BUFFER_SIZE];

	while (recv(ConnectionSocket, RxBuffer, BUFFER_SIZE, 0))
	{
		char* fileBuffer = new char[BUFFER_SIZE];
		
		PktDef currentPacket(RxBuffer);
		currentPacket.displayPacket();

		if (currentPacket.getErrFlag() == false)
		{
			ofstream outputStream;
			outputStream.open("output.txt", ios::app | ios::binary);
			outputStream.write(fileBuffer, BUFFER_SIZE);

			outputStream.close();
		}
		else cout << "Error packet received" << endl;

		PktDef ackPacket;
		ackPacket.setHeaderAckFlag(true);
		RxBuffer = ackPacket.serializePacket();
		send(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
	}
	
	closesocket(ConnectionSocket);	//closes incoming socket
	closesocket(ServerSocket);	    //closes server socket	
	WSACleanup();					//frees Winsock resources

	//This code has been added to simply keep the console window open until you
	//type a character.
	int garbage;
	cin >> garbage;

	return 1;
}