#include "stdafx.h"


string serialize(const string& str){
	int len = str.length();
	const string strlen((char*)&len, 4);
	return strlen + str;
}

string deserialize(const char* str){
	const int len = *(const int*)(str);
	return string(str + 4, len);
}

int Enviar(SOCKET ConnectSocket, char* sendbuf, int longitud){ // return 0 = OK
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	char* auxiliar;
	string ACK;

	////////////////////////////////////////////////////////////////////////////////
	//    Enviar informacion recibida en sendbuf
	////////////////////////////////////////////////////////////////////////////////
	// Send an initial buffer: sendbuf contiene la frase que mandas
	iResult = send(ConnectSocket, sendbuf, longitud, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////////
	//    Esperar ACK
	////////////////////////////////////////////////////////////////////////////////
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			//Recibir
			auxiliar = new char[DEFAULT_BUFLEN+1];
			auxiliar[DEFAULT_BUFLEN] = '\0';
			for (int ii = 0; ii < DEFAULT_BUFLEN; ii++){
				auxiliar[ii] = recvbuf[ii];
			}
			//deserialize:
			ACK = deserialize(auxiliar);
			cout << "ACK Recibido" << endl;
			if (ACK.compare("ACK") == 0){
				break;
			}
			delete[] auxiliar;
		}
		else if (iResult < 0) {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
	} while (true);

	return 0;
}

int Menu(SOCKET ConnectSocket){ 
	int tamaño_msg, iResult, longitud, option, volver;
	char msg[155], borrar[10],follow[40];
	char* enviar_char;
	string enviar;

	////ID////
	cout << "\n*** Cliente ***" << endl << "\n\tIntroduzca ID: ";
	cin.getline(msg, 512);
	enviar = serialize("ID") + serialize(msg);
	//con vertir a char
	enviar_char = new char[enviar.length() + 1];
	enviar_char[enviar.length() - 1] = 0;
	for (int ii = 0; ii < enviar.length(); ii++){ enviar_char[ii] = enviar[ii]; }
	//longitud del mensaje:
	longitud = enviar.length();
	//enviar ID y esperar ack dentro de la funcion
	Enviar(ConnectSocket, enviar_char, longitud);
	delete[] enviar_char;

	//mensajes
	while (true){
		cout << "\n Introduce the desired function: \n1.Publish message \n2.List of messages published by my usser \n3.Delete one of your messages \n4.Follow or unfollow an usser \n5.list of messages of followed people: ";
		cin >> option;
		if (option==1)
		{
		cout << "\n\n\tIntroduzca mensaje: ";
		cin.ignore();
		cin.getline(msg, 512);
		if (strlen(msg) > 0){  //WRITELINE
			enviar = serialize("WRITELINE") + serialize(msg);
			enviar_char = new char[enviar.length() + 1];
			enviar_char[enviar.length() - 1] = 0;
			for (int ii = 0; ii < enviar.length(); ii++){ enviar_char[ii] = enviar[ii]; }
			longitud = enviar.length();
			//enviar ID y esperar ack dentro de la funcion
			Enviar(ConnectSocket, enviar_char, longitud);
			delete[] enviar_char;
		}
		else if (strlen(msg) == 0){
			// END
			cout << "\nEND\n";
			enviar = serialize("END");
			enviar_char = new char[enviar.length() + 1];
			enviar_char[enviar.length() - 1] = 0;
			for (int ii = 0; ii < enviar.length(); ii++){ enviar_char[ii] = enviar[ii]; }
			longitud = enviar.length();
			//enviar ID y esperar ack dentro de la funcion
			Enviar(ConnectSocket, enviar_char, longitud);
			delete[] enviar_char;
			break;
		}
		}
		//print my timeline
		else if(option==2)
		{
			while(1)
			{
			if(volver==1)
			{
				enviar = serialize("printme");
				enviar_char = new char[enviar.length() + 1];
				enviar_char[enviar.length() - 1] = 0;
				for (int ii = 0; ii < enviar.length(); ii++){ enviar_char[ii] = enviar[ii]; }
				longitud = enviar.length();
				//enviar ID y esperar lista de mensajes dentro de la funcion con el ID del mensaje
				Enviar(ConnectSocket, enviar_char, longitud);
				delete[] enviar_char;

				//si se quiere más se pone un 1
				
			}
			else if (volver==2)
			{
				break;
			}
			else
			{
				cout<<" Si se desean más mesajes pulse 1, si quiere volver al menu pulse el 2";
				cin>> volver;
			}
			}
			
		}
		//delete a message
		else if(option==3)
		{
			cout<<"which message do you want to delete?";
			//mostrar mensajes
			while(1)
			{
			if(volver==1)
			{
				enviar = serialize("printme");
				enviar_char = new char[enviar.length() + 1];
				enviar_char[enviar.length() - 1] = 0;
				for (int ii = 0; ii < enviar.length(); ii++){ enviar_char[ii] = enviar[ii]; }
				longitud = enviar.length();
				//enviar ID y esperar lista de mensajes dentro de la funcion con el ID del mensaje
				Enviar(ConnectSocket, enviar_char, longitud);
				delete[] enviar_char;

				//si se quiere más se pone un 1
				cout<<" Si se desean más mesajes pulse 1, si quiere volver al menu pulse el 2";
				cin>> volver;
				
			}
			else if (volver==2)
			{
				break;
			}
			else
			{
				cout<<" Si se desean más mesajes pulse 1, si quiere volver al menu pulse el 2";
				cin>> volver;
			}
			}
			cin.getline(borrar,10);
			//envio del numero para borrarlo
				enviar = serialize("borrar")+serialize(borrar);
				enviar_char = new char[enviar.length() + 1];
				enviar_char[enviar.length() - 1] = 0;
				for (int ii = 0; ii < enviar.length(); ii++){ enviar_char[ii] = enviar[ii]; }
				longitud = enviar.length();
				//enviar ID y esperar lista de mensajes dentro de la funcion con el ID del mensaje
				Enviar(ConnectSocket, enviar_char, longitud);
				delete[] enviar_char;

		}
		//follow or unfollow an user
		else if(option==4)
		{
			//mostrar mensajes
			while(1)
			{
			if(volver==1)
			{
				enviar = serialize("print_users");
				enviar_char = new char[enviar.length() + 1];
				enviar_char[enviar.length() - 1] = 0;
				for (int ii = 0; ii < enviar.length(); ii++){ enviar_char[ii] = enviar[ii]; }
				longitud = enviar.length();
				//enviar ID y esperar lista de users dentro de la funcion 
				Enviar(ConnectSocket, enviar_char, longitud);
				delete[] enviar_char;

				//si se quiere más se pone un 1
				cout<<" Si se desean más usuarios pulse 1, si quiere volver al menu pulse el 2";
				cin>> volver;
				
			}
			else if (volver==2)
			{
				break;
			}
			else
			{
				cout<<" Si se desean más mesajes pulse 1, si quiere volver al menu pulse el 2";
				cin>> volver;
			}
			}
			//fataria comprobar en el servidor si el usuario ya le sigue entonces es unfollow
			cin.getline(follow,40);
			enviar = serialize("print_users");
			enviar_char = new char[enviar.length() + 1];
			enviar_char[enviar.length() - 1] = 0;
			for (int ii = 0; ii < enviar.length(); ii++){ enviar_char[ii] = enviar[ii]; }
			longitud = enviar.length();
			//enviar ID y esperar lista de users dentro de la funcion 
			Enviar(ConnectSocket, enviar_char, longitud);
			delete[] enviar_char;

		}
		//list o messages of the people y follow
		else if (option==5)
		{
			enviar = serialize("messages_follow");
			enviar_char = new char[enviar.length() + 1];
			enviar_char[enviar.length() - 1] = 0;
			for (int ii = 0; ii < enviar.length(); ii++){ enviar_char[ii] = enviar[ii]; }
			longitud = enviar.length();
			//enviar ID y esperar lista de mensajes que sigo dentro de la funcion 
			Enviar(ConnectSocket, enviar_char, longitud);
			delete[] enviar_char;
		}
	}
	// shutdown the connection since no more data will be sent
	printf("\nConnection closing...\n");
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}


	return 0;
}

int __cdecl main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		*ptr = NULL,
		hints;
	char* sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	Menu(ConnectSocket);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}