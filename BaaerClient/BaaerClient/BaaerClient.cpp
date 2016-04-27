#include "stdafx.h"

// These variables are defined in a global manner so that they can be accessed by the functions without the addition of unnecessary variables
int argc;
char **argv;
string username;
int pagecount;
bool newpages;

// The following function enables us to prepare the information for a transmission between the client and server by serializing the vector into an array of characters
string serialize(const string& str){
	int len = str.length();
	const string strlen((char*)&len, 4);
	return strlen + str;
}

// This function performs the opposite operation of the serialize function defined previously. It obtains the original vector from the array of characters
string deserialize(const char* str){
	const int len = *(const int*)(str);
	return string(str + 4, len);
}

//  This function is the one employed for sending information between the client and the server (the actual baa)
int send_msg(SOCKET ConnectSocket, char* sendbuf, int longitud){ // return 0 = OK
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	char* auxiliar;
	string ACK;
  int ServerOk = 1;

	////////////////////////////////////////////////////////////////////////////////
	//    TRANSMISSION PROCESS
	////////////////////////////////////////////////////////////////////////////////
	//  An initial buffer is sent
	////////////////////////////////////////////////////////////////////////////////

	iResult = send(ConnectSocket, sendbuf, longitud, 0); // sendbuf contains the information being sent

	if (iResult == SOCKET_ERROR) {
		printf("send function failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////////
	//    Await the ACK
	////////////////////////////////////////////////////////////////////////////////

	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			// Reception
			auxiliar = new char[DEFAULT_BUFLEN+1];
			auxiliar[DEFAULT_BUFLEN] = '\0';
			for (int ii = 0; ii < DEFAULT_BUFLEN; ii++){
				auxiliar[ii] = recvbuf[ii];
			}
			
			int len=0;
			ACK = deserialize(auxiliar);
			len=ACK.length()+4;
			if(ACK.compare("mybaas")==0){
        ACK = deserialize(auxiliar +len);
        int number=stoi(ACK);
        if(number == 0){
          cout << "There are no Baas to display" << endl;
          newpages = 0;
        }else{
				  for(int i=0;i<number;i++)
				  {
					  len=len+ ACK.length()+4;
					  ACK = deserialize(auxiliar +len );
					  cout << ACK<<". ";
					  len=len+ ACK.length()+4;
					  ACK = deserialize(auxiliar +len );
					  cout << ACK << " ";
            len=len+ ACK.length()+4;
            ACK = deserialize(auxiliar +len );
            cout << ACK<<endl;
				  }
          len=len+ ACK.length()+4;
          ACK = deserialize(auxiliar +len );
          if(ACK.compare("1")==0){
            newpages = 1;
          }else{
            newpages = 0;
          }
        }
        break;
			}else if(ACK.compare("timeline")==0){
			  ACK = deserialize(auxiliar + len);
			  int number=stoi(ACK);
        if(number == 0){
          cout << "There are no Baas to be displayed..." << endl;
          newpages = 0;
        }
		    else{
				  for(int i=0;i<number;i++)
				  {
					  len=len+ ACK.length()+4;
					  ACK = deserialize(auxiliar +len);
					  len=len+ ACK.length()+4;
					  ACK = deserialize(auxiliar +len);
					  cout << ACK << " ";
            len=len+ ACK.length()+4;
            ACK = deserialize(auxiliar +len);
            cout << "@" << ACK << ": ";
            len=len+ ACK.length()+4;
            ACK = deserialize(auxiliar +len);
            cout << ACK<<endl;
				  }
          len=len+ ACK.length()+4;
          ACK = deserialize(auxiliar +len);
          if(ACK.compare("1")==0){
            newpages = 1;
          }else{
            newpages = 0;
          }
        }
				  break;
			}
			else
			{
			  ACK = deserialize(auxiliar + ACK.length() + 4);
			  if (ACK.compare("0") == 0)
			  { // A 1 value represents an error, a 0 value represents a correct execution
				  ServerOk = 0; // Petition has been correctly processed
				  break;
			  }
			  else if(ACK.compare("2") == 0)
			  {
				  ServerOk = 2; // petition processed correctly
			  break;
			  }
			  else
			  {
				  return 1;
			  }
			}
			delete[] auxiliar;
		}
		else if (iResult < 0)
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
	} 
	while (true);

  	// Cleanup process (Winsock Protocol)
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
	if (iResult != 0) 
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) 
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one connection successfully takes place

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// A SOCKET is generated in order to connect to the Server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) 
		{
			printf("socket failed with an error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connection to the Server
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) 
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) 
	{
		printf("Unable to sucessfully connect to the server!\n");
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

// The following function is employed to setting up/initializing a user

bool set_user(){
	string message;
	int isOk;

	cout << "Please introduce a username (Maximum allowable length = 30 characters): ";
	getline(cin,username);
	if(username.length()<=30)
	{
		message =serialize("0") + serialize(username);
		isOk = prepare_send(message);
	}
	else
	{
		cout<<" Username exceeds the maximum allowable length. Please choose a new one."<<endl;
		isOk=1;
	}
  
	return isOk;
};

// The following function is employed to create a new baa
bool new_baa(string user){
	string message, baa;
	bool isOk;
	int max_input;
	cout << "Maximum allowable length for a baa = 140 characters: ";
	cin.ignore();cin.clear();
	getline(cin,baa);
	if(baa.length()<=140)
	{
		message =serialize("1") + serialize(username) + serialize(baa);
 
		isOk = prepare_send(message);
	}
	else
	{
		cout<<" Baa exceeds the maximum allowable length. Please rewrite it."<<endl;
		isOk=1;
	}
  
	return isOk;
};

// This function allows the user to request his own timeline, in other words, to ask the server to provide him/her with his own baas

void my_baas(string user, int PageCounter)
{
	string message;
	bool isOk;
	int iResult;
  char *newpage=new char;
  itoa(PageCounter,newpage,10);
	message =serialize("2") + serialize(username) + serialize(newpage);
	isOk = prepare_send(message);
}

// This function is employed by the user to delete one of his/her previous baas

bool unbaa(string user){
	string message;
	char *baaIdChar = new char;
	unsigned int baaId;
	bool isOk;
  int keep;
  char nextpage = 'i';
	cout<<"This is your timeline "<<endl;
  keep = 1;
  pagecount = 0;
  while(keep == 1){
    my_baas(user,pagecount);
    if(newpages){
      cout << "Press n to see another page, other key to choose a baa from this page" << endl;
      cin >> nextpage;
    }
    if(nextpage == 'n'){
      ++pagecount;
      nextpage = 'i';
    }else{
      keep = 0;
    }
  }
	cout << "Introduce the id of the Baa to be deleted: ";
	cin.ignore();cin.clear();
	cin >> baaId;
	itoa (baaId,baaIdChar,10);
	message =serialize("3") + serialize(username) + serialize(baaIdChar);
	isOk = prepare_send(message);
	return isOk;
};

// This function allows a user to "follow" another one
void follow(string user){
	string message,follow;
	short int isOk;
	cout << "Username to (Un)follow: ";
	cin.ignore();cin.clear();
	getline(cin,follow);
	message =serialize("4") + serialize(username) + serialize(follow);
	isOk = prepare_send(message);
	if(isOk == 0)
	{
		cout << "You are now following @" << follow << endl;
	}
	else if(isOk == 1){
    cout << "You are now unfollowing @" << follow << endl;
	}
	else if(isOk == 2)
	{
    cout << "Sorry, user @" << follow << " doesn't exists" << endl;
	}
	else
	{
    cout << "Unexpected error, please retry" << endl;
  }
};

// This function allows a user to see the timeline of the people followed by him

void timeline(string user, int PageCounter)
{
	string message;
	bool isOk;
	int iResult;
  char *newpage=new char;
  itoa(PageCounter,newpage,10);
	message =serialize("5") + serialize(username) + serialize(newpage);
	isOk = prepare_send(message);
}

// This function allows a user to logout from the system correctly in order to be able to log in again

bool log_out(string user)
{
	string message;
	int isOk;
	message =serialize("6") + serialize(username);
	isOk = prepare_send(message);
	return isOk;
};

int __cdecl main(int ac, char** av) {
	// Copy ac and av to the global variable
	argc = ac;
	argv = av;
	int option, keep,pagecount;
	bool check;
  char nextpage;
  nextpage = 'i';

	cout << "\n*** Client ***" << endl;

  //Set username
	while(1){
    check = set_user();
    if(check==0)
	{
      check = 1;
      break;
    }
	else
	{
      cout << "Error" << endl;
    }
}
	cout << "Welcome " << username << endl;
	
	// MENU DISPLAY 
	while(1)
	{
		cout << endl << "*******************************************" << endl << "Main menu" << endl ;
		cout << "1. Baa" << endl << "2. My Baas" << endl << "3. UnBaa" << endl << "4. Follow/Unfollow" << endl << "5. Baas timeline" << endl << "6. Exit" << endl << "*******************************************" << endl;
		cin >> option;
		if(option==1){
			check = new_baa(username);
		if(check == 0){
			cout << "Baa sent correctly";
		}
		else
		{
			cout << "An error has been produced";
		}
	}
		else if(option==2)
		{
      keep = 1;
		  pagecount = 0;
      while(keep == 1){
        my_baas(username,pagecount);
        if(newpages){
          cout << "Press n to see another page, other key to finish" << endl;
          cin >> nextpage;
        }
        if(nextpage == 'n'){
          ++pagecount;
          nextpage = 'i';
        }else{
          keep = 0;
        }
      }
		}
		else if(option==3)
		{
		check = unbaa(username);
		if(check == 0)
		{
			cout << "Baa deleted correctly";
		}
		else
		{
        cout << "Error: Baa not found or not enough permissions";
		}  
    }
		else if(option==4)
		{
		follow(username);
		}
		else if(option==5)
		{
      keep = 1;
		  pagecount = 0;
      while(keep == 1){
        timeline(username,pagecount);
        if(newpages){
          cout << "Press n to see another page, other key to finish" << endl;
          cin >> nextpage;
        }
        if(nextpage == 'n'){
          ++pagecount;
          nextpage = 'i';
        }else{
          keep = 0;
        }
      }
		}
		else if(option==6)
		{
		check = log_out(username);
			if(check == 0)
			{
				cout << "Shutting Down the Baaer Client" << endl;
				break;
			}
			else
			{
				cout << "Sorry, an error occurred logging out, please retry..." << endl;
			}
		}
		else
		{
			cout << "Please, introduce a valid option" << endl;
		}
  }

	return 0;
}