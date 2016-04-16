#include "stdafx.h"

SOCKET ClientSocket = INVALID_SOCKET;
SOCKET ListenSocket = INVALID_SOCKET;

mutex logFileMutex;
ofstream logFile;

unsigned int last_id = 0;
//definiciones de donde se guardan las cosas solo falta que ademas de llegar se guarden las cosas en donde tienten que estar
/** Class *****************************************************/
/*
 * This struct will contain all the information regarding a message
 */
struct Messages {
  unsigned int id;
  string user_name;
  string content;
  time_t timestamp;
};

struct Users
{
  string username;
  bool loggedIn;
  map<unsigned int,bool> messages;
};

//map<string,time_t> LoggedIn;
map<unsigned int, Messages> Global_messages;
map<string,Users> Global_users;

//map<string,time_t>::iterator itLogged = LoggedIn.begin();

bool addLogged(string username){ // 0 OK, 1 error
  if(Global_users.find(username) != Global_users.end()){
    if(Global_users[username].loggedIn = 0){
      Global_users[username].loggedIn = 1;
      return 0;
    }else{
      return 1;
    }
  }else{
    Users tempUser;
    tempUser.username = username;
    tempUser.loggedIn = 1;
    Global_users.insert(pair<string,Users>(username,tempUser));
    return 0;
  }
}

bool newBaa(Messages baaData){
  Global_messages.insert(pair<int,Messages>(baaData.id,baaData));
  Global_users[baaData.user_name].messages.insert(pair<unsigned int, bool>(baaData.id,0));
  return 0;
}

void myBaa(){
}

void unBaa(){
}

void Follow(){
}

void TimeBaas(){
}

//vector de threads definicion, para hacer algo meter

vector <thread> threads;

//global messages[ident].
int print_timeline(const string& user_name, bool show_id)
{
  int count = 0;

  cout << endl << "----------------" << endl;
  if (user_name.empty()) { // all messages, no filter
    for (int i=Global_messages.size()-1; i ==0 ; --i) {
      if (show_id) { // print ID of message
        cout << Global_messages[i].id << " ";
      }
      cout << "[@" << Global_messages[i].user_name << "]: " << Global_messages[i].content << endl;
    }
    count = Global_messages.size();
  } else { // specific user
    for (int i=Global_messages.size()-1; i ==0 ; --i) {
      if (Global_messages[i].user_name == user_name) {
        if (show_id) { // print ID of message
          cout << Global_messages[i].id << " ";
        }
        cout << "[@" << user_name << "]: " << Global_messages[i].content << endl;
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
	int iResult, iSendResult, doneInt;
	char recvbuf[DEFAULT_BUFLEN];
	char *temporal, *ACK_char;
	int recvbuflen = DEFAULT_BUFLEN;
	string user, type, data, ACK, doneStr;
    Messages tempMessage;

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
        if(type == "0"){
			    //user and time of conection
			    user = deserialize(temporal + 5);
			    cout << "\n\nUsername: " << user;
			    doneInt = addLogged(user);
			    doneStr = to_string(doneInt);
        }else if(type == "1"){
          tempMessage.user_name = deserialize(temporal + 5);
          cout << "\n\nUsername: " << tempMessage.user_name << endl;
          tempMessage.content = deserialize(temporal + 5 + tempMessage.user_name.length() + 4);
          tempMessage.id = last_id; ++last_id;
          tempMessage.timestamp = time(0);
          doneInt = newBaa(tempMessage);
          doneStr = to_string(doneInt);
        }else if(type == "2"){
			    //My baas
        }else if(type == "3"){
			    //Unbaa
				 
        }else if(type == "4"){
			      //Follow/Unfollow an usser
		        user = deserialize(temporal + 5);
		        cout << "\n\nUsername: " << user;
            data = deserialize(temporal + 5 + user.length() + 4);
            cout << data;
        }else if(type == "5"){
			    //Baas timeline
        }else if(type == "6"){
			    //Exit
        }else{
          cout << "Error" << endl;
          // Create error code
        }
				delete[] temporal; 

			////////////////////////////////////////////////////////////////////////////////
			//    Enviar ACK
			////////////////////////////////////////////////////////////////////////////////
			// Send an initial buffer: sendbuf contiene la frase que mandas
			ACK = serialize("ACK") + serialize(doneStr);
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
  closesocket(ClientSocket);
	return 0;
}

int set_up_server(int error){
	WSADATA wsaData;
	int iResult;

  error = 0;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		error = 1;
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
		error = 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		error = 1;
	}

	// Setup the TCP listening socket
	iResult = ::bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		error = 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		error = 1;
	}
  return error;
}

void ThreadFunction(SOCKET ClientSocket){
	receive(ClientSocket);
}

int __cdecl main(void) {

  int result = 1;

  while(result!=0){
    result = set_up_server(result);
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
    /*
     añadimos un nuevo thread si hay un nuevo client socket,
    */
		threads.push_back(thread(ThreadFunction, ClientSocket));
		//el detach es para que los threads puedan correr en paralelo, siendo estos los que hacen la verdadera paralezación de codigo.
		threads[threads.size()-1].detach();
	}
	
	// cleanup
  closesocket(ListenSocket);
	WSACleanup();
	return 0;
}