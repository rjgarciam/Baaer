#include "stdafx.h"

SOCKET ClientSocket = INVALID_SOCKET;
SOCKET ListenSocket = INVALID_SOCKET;

unsigned int last_id = 0;
/*
 * The following struct will contain all the necessary information regarding the messages that are going to be handled
 */
struct Messages {
  unsigned int id;
  string user_name;
  string content;
  time_t timestamp;
};
/*
 * The following struct will contain all the necessary information regarding the Users that are using our application
 */
struct Users
{
  string username;
  bool loggedIn;
  map<unsigned int,bool> messages;
  map<string,bool> follows;
  int following;
};

map<unsigned int, Messages> Global_messages;      //This the way in which we chose to implement our Map to all Messages
map<string,Users> Global_users;                   //This the way in which we chose to implement our Map to all Users
mutex iniciate_Mutex;
vector <thread> threads;                          //This the way in which we chose to implement our threads vector

// The following function enables us to prepare the information for a transmission between the client and server by serializing the vector into an array of characters
string serialize(const string& str)
{
	int len = str.length();
	const string strlen((char*)&len, 4);
	return strlen + str;
}

// This function performs the opposite operation of the serialize function defined previously. It obtains the original vector from the array of characters
string deserialize(const char* str)
{
	const int len = *(const int*)(str);
	return string(str + 4, len);
}

// The following function is employed to setting up/initializing a user
bool addLogged(string username)
{ // A 0 value represents that the operation has taken place successfully, a 1 represents an error
  if(Global_users.find(username) != Global_users.end())
  {
    if(Global_users[username].loggedIn == 0)
	{
      Global_users[username].loggedIn = 1;
      return 0;
    }
	else
	{
      return 1;
    }
  }
  else
  {
    Users tempUser;
    tempUser.username = username;
    tempUser.loggedIn = 1;
    Global_users.insert(pair<string,Users>(username,tempUser));
    return 0;
  }
}

// The following function is employed to create a new baa
bool newBaa(Messages baaData)
{
  iniciate_Mutex.lock();
  Global_messages.insert(pair<int,Messages>(baaData.id,baaData));
  Global_users[baaData.user_name].messages.insert(pair<unsigned int, bool>(baaData.id,0));
  iniciate_Mutex.unlock();
  return 0;
}

// This function allows the user to request his own timeline, in other words, to ask the server to provide him/her with his own baas
string print_timeline(const string& user_name, int PagID)
{
  map<unsigned int, Messages>::reverse_iterator rit;
  int count = 0, countpage = 0;
  int newpage = 0;
  string msg;
  char timebuff[20];
  char *cont=new char;
  char *page=new char;
  char* indent= new char;
  int length=Global_messages.size();
  cout << endl << "Printing " << user_name << " Baas" << endl;
  
  for (rit=Global_messages.rbegin(); rit!=Global_messages.rend(); ++rit) {
    if (rit->second.user_name == user_name) {
		  itoa(rit->second.id,indent,10);
		  string iden=string(indent);
		  ++count;
      if(count > (PagID * 4)){
        strftime(timebuff, 20, "%Y-%m-%d", localtime(&rit->second.timestamp));
        cout << timebuff << endl;
        msg=msg+serialize(indent )+ serialize(timebuff) + serialize( rit->second.content );
        ++newpage;
        ++countpage;
        PagID = 0;
        if(newpage == 4){
          newpage = 1;
          PagID = 1;
          break;
        }
      }
	  }
	}
	itoa(countpage,cont,10);
  itoa(PagID,page,10);
	msg=serialize(cont)+ msg +serialize(page);
  return msg;
}

// This function is employed by the user to delete one of his/her previous baas
bool unBaa(string username,string baaIdChar)
{
  unsigned int baaId;
  baaId = stoi(baaIdChar);
  if(Global_users[username].messages.find(baaId) != Global_users[username].messages.end())
  {
    iniciate_Mutex.lock();
    Global_users[username].messages.erase(baaId);
    Global_messages.erase(baaId);
    iniciate_Mutex.unlock();
    return 0;
  }
  else
  {
    return 1;
  }
}

// This function works in the following way:
//	--> A 0 value is used to insert/begin following someone
//	--> A 1 value is used to erase/stop following someone
//	--> A 2 value states that there no user exists that is employing that username
int Follow(string username,string follow)
{ 
  if(Global_users.find(follow) != Global_users.end())
  {
    if(Global_users[username].follows.find(follow) != Global_users[username].follows.end())
	{
      map<string,bool>::iterator itFollow;
      itFollow = Global_users[username].follows.find(follow);
      iniciate_Mutex.lock();
      Global_users[username].follows.erase(itFollow);
      Global_users[username].following--;
      iniciate_Mutex.unlock();
      return 1;
    }
	else
	{
      iniciate_Mutex.lock();
      Global_users[username].follows.insert(pair<string,bool>(follow,0));
      Global_users[username].following++;
      iniciate_Mutex.unlock();
      return 0;
    }
  }
  else
  {
    return 2;
  }
}

// This function allows a user to see the timeline of the people followed by him
string timeline(const string& username, int PagID)
{
  map<unsigned int, Messages>::reverse_iterator rit;
  int count = 0, countpage = 0, newpage = 0;
  string msg, folo;
  char timebuff[20];
  char *cont=new char;
  char *page=new char;
  char* indent= new char;
  int length=Global_messages.size();
  cout << endl << "Printing " << username << " timeline" << endl;
  for (rit=Global_messages.rbegin(); rit!=Global_messages.rend(); ++rit) 
  {
		folo=rit->second.user_name;

			if (Global_users[username].follows.find(folo) != Global_users[username].follows.end())
			{
				itoa(rit->second.id,indent,10);
				string iden=string(indent);
		    ++count;
        if(count > (PagID * 4)){
          strftime(timebuff, 20, "%Y-%m-%d", localtime(&rit->second.timestamp));
          cout << timebuff << endl;
          msg=msg+serialize(indent )+ serialize(timebuff) + serialize( rit->second.user_name ) + serialize( rit->second.content );
          ++newpage;
          ++countpage;
          PagID = 0;
          if(newpage == 4){
            newpage = 1;
            PagID = 1;
            break;
          }
        }
			}
  }
	itoa(countpage,cont,10);
  itoa(PagID,page,10);
	msg=serialize(cont)+ msg +serialize(page);

  return msg;
}

// 0 --> Logged out successfully	1 --> Error has occurred
bool log_out(string username)
{ 
  if(Global_users.find(username) != Global_users.end())
  {
      Global_users[username].loggedIn = 0;
      return 0;
   }
  else
  {
      return 1;
  }
}

int enviar(const string& mensaje)
{
	int iResult, longitud;
	char* send_char;
	send_char = new char[mensaje.length() + 1];
	longitud=mensaje.length();
  iniciate_Mutex.lock();
	for (int ii = 0; ii < mensaje.length(); ii++)
	{ 
		send_char[ii] = mensaje[ii]; 
	}
	iResult = send(ClientSocket, send_char, longitud, 0);
	delete[] send_char;
  iniciate_Mutex.unlock();
	return iResult;
}

int receive(SOCKET ClientSocket){
	int iResult, iSendResult, doneInt,pageID;
	char recvbuf[DEFAULT_BUFLEN];
	char *temporal, *ACK_char;
	int recvbuflen = DEFAULT_BUFLEN;
	string user, type, data, ACK, doneStr,msg;
    Messages tempMessage;

	// Reception process is maintained until the peer shuts down the connection

	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			////////////////////////////////////////////////////////////////////////////////
			//    Received data is introduced into recbuf 
			////////////////////////////////////////////////////////////////////////////////

				temporal = new char[DEFAULT_BUFLEN + 1];
				temporal[DEFAULT_BUFLEN] = '\0';
				for (int ii = 0; ii < DEFAULT_BUFLEN; ii++)
				{
					temporal[ii] = recvbuf[ii];
				}

				size_t len = strlen(temporal);
				char* nueva = new char[len+ 1];
				for (int ii = 0; ii < (len+1); ii++)
				{
				nueva[ii] = temporal[ii];
				}

				type = deserialize(temporal);
				if(type == "0")
				{

			    //A user logs in

			    user = deserialize(temporal + 5);
			    cout << "Username: @" << user << " has logged in" << endl;
          iniciate_Mutex.lock();
			    doneInt = addLogged(user);
          iniciate_Mutex.unlock();
			    doneStr = to_string(doneInt);
        }else if(type == "1"){
          //A new Baa is sent
          tempMessage.user_name = deserialize(temporal + 5);
          cout << "Username: @" << tempMessage.user_name << " has Baaed" << endl;
          tempMessage.content = deserialize(temporal + 5 + tempMessage.user_name.length() + 4);
          tempMessage.id = last_id; ++last_id;
          tempMessage.timestamp = time(0);
          doneInt = newBaa(tempMessage);
          doneStr = to_string(doneInt);
        }else if(type == "2"){
			    //My baas
			    user = deserialize(temporal + 5);
          pageID = stoi(deserialize(temporal + 5 + user.length() + 4));
			    msg=print_timeline(user,pageID);     
        }else if(type == "3"){
          //Baa elimination --> Unbaa
          user = deserialize(temporal + 5);
          //cout << "Username: " << user << " is removing a tweet";
          data = deserialize(temporal + 5 + user.length() + 4);
          //cout << data;
          doneInt = unBaa(user,data);
          doneStr = to_string(doneInt);
          if(doneInt == 0){
            cout << "User: @" << user << " has removed a tweet with ID: " << data << endl;
          }else{
            cout << "The user @" << user << " has received an error trying to remove a tweet with ID: " << data << endl;
          }
        }else if(type == "4"){
          //Follow/Unfollow an usser
          user = deserialize(temporal + 5);
          //cout << "\n\nUsername: " << user;
          data = deserialize(temporal + 5 + user.length() + 4);
          //cout << data;
          doneInt = Follow(user,data);
          doneStr = to_string(doneInt);
          if(doneInt == 0){
            cout << "User: @" << user << " has started following  @" << data << endl;
          }else if(doneInt == 1){
            cout << "The user @" << user << " has stopped following  @" << data << endl;
          }else{
            cout << "The user @" << user << " has received an error trying to follow @" << data << endl;
          }
        }else if(type == "5"){
          //Baas timeline
          user = deserialize(temporal + 5);
          pageID = stoi(deserialize(temporal + 5 + user.length() + 4));
          //cout << "\n\nUsername: " << user;
          msg=timeline(user,pageID);
        }else if(type == "6"){
          user = deserialize(temporal + 5);
			    doneInt = log_out(user);
          doneStr = to_string(doneInt);
          if(doneInt == 0){
            cout << "User: @" << user << " has logout"<< endl;
          }else{
            cout << "User @" << user << " has received an error trying to logout" << endl;
          }
        }else{
          cout << "Error" << endl;
          // Create error code
        }
				delete[] temporal; 

			////////////////////////////////////////////////////////////////////////////////
			//    Send ACK
			////////////////////////////////////////////////////////////////////////////////

			// Send an initial buffer: sendbuf contiene la frase que mandas
			if (type=="2"){
				ACK =serialize("mybaas")+msg;
			}else if (type=="5"){
				ACK =serialize("timeline")+msg;
			}else{
			  ACK = serialize("ACK") + serialize(doneStr);
			}
			iResult=enviar(ACK);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
		}
		else if (iResult == 0) 
		{
			printf("Closing connection...\n");
		}
		else  
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} 
	while (iResult > 0);

	// Connection is shut down since the process is over

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) 
	{
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
	if (iResult != 0) 
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		error = 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Server adress and port resolution

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);

	if (iResult != 0) 

	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		error = 1;
	}

	// Create a SOCKET for connecting to server
  iniciate_Mutex.lock();
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  iniciate_Mutex.unlock();
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		error = 1;
	}

	// Setup the TCP listening socket

  iniciate_Mutex.lock();
	iResult = ::bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
  iniciate_Mutex.unlock();
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		error = 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) 
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		error = 1;
	}
  return error;
}

void ThreadFunction(SOCKET ClientSocket)
{
	receive(ClientSocket);
}

int __cdecl main(void) 
{

  int result = 1;

  while(result!=0)
  {
    result = set_up_server(result);
  }
  printf("Awaiting a connection...\n");


   while (true)
   {
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) 
		{
			printf("An error has occured while attempting to accept the socket: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		printf("New connection accepted\n");
    /*
     If a new client socket is generated, a new thread is created to handle it
    */
		threads.push_back(thread(ThreadFunction, ClientSocket));
		// The detach utility tool allows the threads to be run in parallel, since the threads themselves are the ones that actually parallelize the code 
		threads[threads.size()-1].detach();
    for(auto ct=threads.begin(); ct!=threads.end(); ++ct){
      if(ct->joinable() == false){
        iniciate_Mutex.lock();
        threads.erase(ct);
        iniciate_Mutex.unlock();
        break;
      }
    }
	}
	
	// cleanup
	closesocket(ListenSocket);
	WSACleanup();
	return 0;
}