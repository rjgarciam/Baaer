#include "stdafx.h"


mutex logFileMutex;
ofstream logFile;
//definiciones de donde se guardan las cosas solo falta que ademas de llegar se guarden las cosas en donde tienten que estar
/** Class *****************************************************/
/*
 * This struct will contain all the information regarding a message
 */
struct Messages {
  static unsigned int s_last_id; // keep track of IDs to assign it automatically
  unsigned int id;
  string user_name;
  string content;

  int year;
  int month;
  int day;
  
  int hour;
  int min;
  int seg;

  Messages(const string& a_user_name, const string& a_content) :
    user_name(a_user_name), content(a_content), id(++s_last_id)
  {
  }
};
list<Messages*> g_messages;
map<unsigned int, list<Messages*>::iterator> g_messages_by_index;
struct users
{
	string ID;
	struct mensajes
	{
		int id_mensaje;
	};
	struct persona
	{
		string identifier;
	};
};
unsigned int Messages::s_last_id = 0;

list<users*> g_users;

//////////////////////
////CAMBIAR POR PARSER
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
//funcion para imprimir el timeline cambiar los cout por contenido a enviar y asi enviamos eso
int print_timeline(const string& user_name, bool show_id)
{
  int count = 0;

  cout << endl << "----------------" << endl;
  if (user_name.empty()) { // all messages, no filter
    for (auto it = g_messages.begin(); it != g_messages.end(); ++it) {
      if (show_id) { // print ID of message
        cout << (*it)->id << " ";
      }
      cout << "[@" << (*it)->user_name << "]: " << (*it)->content << endl;
    }
    count = g_messages.size();
  } else { // specific user
    for (auto it = g_messages.begin(); it != g_messages.end(); ++it) {
      if ((*it)->user_name == user_name) {
        if (show_id) { // print ID of message
          cout << (*it)->id << " ";
        }
        cout << "[@" << user_name << "]: " << (*it)->content << endl;
        ++count;
      }
    }
  }
  cout << "----------------" << endl;

  return count;
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

int receive(SOCKET ClientSocket){ //return 0 = OK
	int iResult, iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	char *temporal, *ACK_char;
	int recvbuflen = DEFAULT_BUFLEN;
	string user, type, data, ACK;

	// Receive until the peer shuts down the connection
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			////////////////////////////////////////////////////////////////////////////////
			//    recibir datos y volcarlos en recvbuf
			////////////////////////////////////////////////////////////////////////////////
				temporal = new char[DEFAULT_BUFLEN + 1];
				temporal[DEFAULT_BUFLEN] = '\0';
				for (int ii = 0; ii < DEFAULT_BUFLEN; ii++){
					temporal[ii] = recvbuf[ii];
				}

        size_t len = strlen(temporal);
        char* nueva = new char[len+ 1];
        for (int ii = 0; ii < (len+1); ii++){
					nueva[ii] = temporal[ii];
				}

				//deserialize:
				type = deserialize(temporal);
        if(type == "1"){
				  user = deserialize(temporal + 5);
				  cout << "\n\nUsername: " << user;
        }else if(type == "2"){
				  user = deserialize(temporal + 5);
				  cout << "\n\nUsername: " << user;
          data = deserialize(temporal + 5 + user.length() + 4);
          cout << data;
        }else if(type == "4"){
        }else if(type == "5"){
        }else if(type == "6"){
        }else if(type == "7"){
        }else{
          cout << "Error" << endl;
          // Create error code
        }
				delete[] temporal; 

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
	receive(ClientSocket);
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