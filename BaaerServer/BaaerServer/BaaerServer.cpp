#include "stdafx.h"
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <mutex>
#include <iostream>
#include <fstream>
#include <thread>
#include <cmath>
#include "windows.h"

using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27010"

mutex logFileMutex;
ofstream logFile;



bool EscribirLog(string DATA, string ID){ //return 1: ok
	//bloquear y abrir archivo para escribir	
	logFileMutex.lock();
	logFile.open("log.txt", std::ofstream::out | std::ofstream::app);
	//formato
	SYSTEMTIME st, lt;
	GetSystemTime(&st);
	time;
	time_t t = time(0);
	struct tm * now = localtime(&t);
	logFile << "[" << ID << "] " << "(" << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << " "
		<< (now->tm_hour) << ':' << (now->tm_min) << ':' << now->tm_sec << "." << st.wMilliseconds << ") " << DATA << endl;
	//cerrar archivo
	logFile.flush();
	logFile.close();
	logFileMutex.unlock();
	
	return 1;
}

string serialize(const string& str){
	int len = str.length();
	const string strlen((char*)&len, 4);
	return strlen + str;
}

string deserialize(const char* str){
	const int len = *(const int*)(str);
	return string(str + 4, len);
}

int Recibir(SOCKET ClientSocket){ //return 0 = OK
	int iResult, iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	char *auxiliar, *ACK_char;
	int recvbuflen = DEFAULT_BUFLEN;
	string ID, TAG, DATA, ACK;
	bool primeraVez = 1;

	// Receive until the peer shuts down the connection
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			////////////////////////////////////////////////////////////////////////////////
			//    recibir datos y volcarlos en recvbuf
			////////////////////////////////////////////////////////////////////////////////
			if (primeraVez == 1){ //ID
				primeraVez = 0;
				auxiliar = new char[DEFAULT_BUFLEN + 1];
				auxiliar[DEFAULT_BUFLEN] = '\0';
				for (int ii = 0; ii < DEFAULT_BUFLEN; ii++){
					auxiliar[ii] = recvbuf[ii];
				}
				//printf("\n\nBytes received: %d\n", iResult);
				//deserialize:
				TAG = deserialize(auxiliar);
				//cout << "TAG: " << TAG;
				if (TAG.compare("END") != 0){
						DATA = deserialize(auxiliar + 4 + TAG.length());
						cout << "\n\nID: " << DATA;
						ID = DATA;
					}
				delete[] auxiliar; //el problema del heap lo da aqui (y lo mismo en la parte del if para WRITELINE)
			}
			else { //WRITELINE
				auxiliar = new char[DEFAULT_BUFLEN+1];
				auxiliar[DEFAULT_BUFLEN] = 0;
				for (int ii = 0; ii < DEFAULT_BUFLEN; ii++){
					auxiliar[ii] = recvbuf[ii];
				}
				//printf("\n\nBytes received: %d\n", iResult);
				//deserialize:
				TAG = deserialize(auxiliar);
				cout << "\n\nID: " << ID << endl;
				cout << "TAG: " << TAG;
				if (TAG.compare("END") != 0){
					DATA = deserialize(auxiliar + 4 + TAG.length());
					cout << "\nDATA: " << DATA;
					EscribirLog(DATA, ID);
				}
				delete[] auxiliar;
			}

			////////////////////////////////////////////////////////////////////////////////
			//    Enviar ACK
			////////////////////////////////////////////////////////////////////////////////
			// Send an initial buffer: sendbuf contiene la frase que mandas
			ACK = serialize("ACK");
			ACK_char = new char[ACK.length() + 1];
			ACK_char[ACK.length() -1] = 0;
			for (int ii = 0; ii < ACK.length(); ii++){ ACK_char[ii] = ACK[ii]; }
			int longitud = ACK.length();

			iResult = send(ClientSocket, ACK_char, longitud, 0);
			delete[] ACK_char;
			cout << "\nACK enviado!";
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			//printf("\nBytes Sent: %ld\n", iResult);
		}
		else if (iResult == 0) {
			printf("\nConnection closing...\n");
		}
		else  {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	return 0;
}

void ThreadFunction(SOCKET ClientSocket){
	Recibir(ClientSocket);
}

int __cdecl main(void) {
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1; 
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = ::bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	printf("Waiting for connection...\n");
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//vaciar log
	logFileMutex.lock();
	logFile.open("log.txt", std::ofstream::out | std::ofstream::trunc);
	logFile.flush();
	logFile.close();
	logFileMutex.unlock();

	while (true){
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		printf("\n\nAccepted!\n");

		thread t(ThreadFunction, ClientSocket);
		t.detach();
	}
	

	// No longer need server socket, we have the CLentSocket
	closesocket(ListenSocket);

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}