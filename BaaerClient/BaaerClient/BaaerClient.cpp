#include "stdafx.h"

// Global variables in order to be able to access from functions without adding parameters
int argc;
char **argv;
string username;


string serialize(const string& str){
	int len = str.length();
	const string strlen((char*)&len, 4);
	return strlen + str;
}

string deserialize(const char* str){
	const int len = *(const int*)(str);
	return string(str + 4, len);
}

int send_msg(SOCKET ConnectSocket, char* sendbuf, int longitud){ // return 0 = OK
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	char* auxiliar;
	string ACK;
  bool ServerOk = 1;

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
	//    Wait for ACK
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
			int len=0;
			ACK = deserialize(auxiliar);
			len=ACK.length()+4;
			if(ACK.compare("timeline")==0)
			{
			ACK = deserialize(auxiliar +len);
			int number=stoi(ACK);
				for(int i=0;i<number;i++)
				{
					len=len+ ACK.length()+4;
					ACK = deserialize(auxiliar +len );
					cout << ACK;
				}
				break;
			}
			else
			{
			cout << "ACK Recibido" << endl;
			cout << ACK;
			ACK = deserialize(auxiliar + ACK.length() + 4);
			cout << ACK;
			if (ACK.compare("0") == 0){ // 0 Ok, 1 error
				ServerOk = 0; // petition processed correctly
        break;
      }else{
        return 1;
      }
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

  	// cleanup
  closesocket(ConnectSocket);
	WSACleanup();
	return ServerOk;
}


SOCKET connect(){
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

return ConnectSocket;
}

int prepare_send(string msg){
  SOCKET ConnectSocket;
  char* send_char;
  int isOk;

  send_char = new char[msg.length() + 1];
	send_char[msg.length() - 1] = 0;
	for (int ii = 0; ii < msg.length(); ii++){ send_char[ii] = msg[ii]; }

  ConnectSocket = connect();
  isOk = send_msg(ConnectSocket, send_char, msg.length());
  return isOk;
}

SOCKET send_conection(string msg)
{
  SOCKET ConnectSocket;
  char* send_char;
  int isOk;

  send_char = new char[msg.length() + 1];
	send_char[msg.length() - 1] = 0;
	for (int ii = 0; ii < msg.length(); ii++){ send_char[ii] = msg[ii]; }

  ConnectSocket = connect();
  isOk = send_msg(ConnectSocket, send_char, msg.length());
  return ConnectSocket;
}
// Function for setting up an user
bool set_user(){
  string message;
  int isOk;

  cout << "Introduce your username: ";
  getline(cin,username);
  message =serialize("0") + serialize(username);
  isOk = prepare_send(message);
  return isOk;
};


bool new_baa(string user){
  string message,baa;
  bool isOk;
  cout << "Baa: ";
  cin.ignore();cin.clear();
  getline(cin,baa);
  message =serialize("1") + serialize(username) + serialize(baa);
  isOk = prepare_send(message);
  return isOk;
};

void my_baas(string user){
  string message;
  bool isOk;
  int iResult;
  message =serialize("2") + serialize(username);
  isOk = prepare_send(message);
}

bool unbaa(string user){
  string message;
  char *baaIdChar = new char;
  unsigned int baaId;
  bool isOk;
  cout << "Introduce the id of the Baa to be deleted: ";
  cin.ignore();cin.clear();
  cin >> baaId;
  itoa (baaId,baaIdChar,10);
  message =serialize("3") + serialize(username) + serialize(baaIdChar);
  isOk = prepare_send(message);
  return isOk;
};

void follow(string user){
  string message,follow;
  bool isOk;
  cout << "Username to (Un)follow: ";
  cin.ignore();cin.clear();
  getline(cin,follow);
  message =serialize("4") + serialize(username) + serialize(follow);
  isOk = prepare_send(message);
  if(isOk == 0){
    cout << "You are now following " << follow << endl;
  }else{
    cout << "You are now unfollowing " << follow << endl;
  }
};
void timeline(string user){};

int __cdecl main(int ac, char** av) {
  // Copy ac and av to the global variable
  argc = ac;
  argv = av;
  int option;
  bool check;

  cout << "\n*** Client ***" << endl;

  //Set username
  while(1){
    check = set_user();
    if(check==0){
      cout << "ok";
      check = 1;
      break;
    }else{
      cout << "Error" << endl;
    }
  }
	cout << "Welcome " << username << endl;
  //Show menu
  while(1){
    cout << endl << "*******************************************" << endl << "Main menu" << endl ;
    cout << "1. Baa" << endl << "2. My Baas" << endl << "3. UnBaa" << endl << "4. Follow/Unfollow" << endl << "5. Baas timeline" << endl << "6. Exit" << endl << "*******************************************" << endl;
    cin >> option;
    if(option==1){
      check = new_baa(username);
      if(check == 0){
        cout << "Baa sent correctly";
      }else{
        cout << "An error has been produced";
      }
    }else if(option==2){
      my_baas(username);
    }else if(option==3){
      check = unbaa(username);
      if(check == 0){
        cout << "Baa deleted correctly";
      }else{
        cout << "Error: Baa not found or not enough permissions";
      }  
    }else if(option==4){
      follow(username);
    }else if(option==5){
      timeline(username);
    }else if(option==6){
      break;
    }else{
      cout << "Please, introduce a valid option" << endl;
    }
  }

	return 0;
}