#include "stdafx.h"

SOCKET ClientSocket = INVALID_SOCKET;
SOCKET ListenSocket = INVALID_SOCKET;

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
  map<string,bool> follows;
  int following;
};

//map<string,time_t> LoggedIn;
map<unsigned int, Messages> Global_messages;
map<string,Users> Global_users;
mutex iniciate_Mutex;
//map<string,time_t>::iterator itLogged = LoggedIn.begin();

bool addLogged(string username){ // 0 OK, 1 error
  if(Global_users.find(username) != Global_users.end()){
    if(Global_users[username].loggedIn == 0){
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


bool unBaa(string username,string baaIdChar){
  unsigned int baaId;
  baaId = stoi(baaIdChar);
  if(Global_users[username].messages.find(baaId) != Global_users[username].messages.end()){
    Global_users[username].messages.erase(baaId);
    Global_messages.erase(baaId);
    return 0;
  }else{
    return 1;
  }
}

int Follow(string username,string follow){ // 0 = Insert; 1 = Erase, 2 = No user with that username
  if(Global_users.find(follow) != Global_users.end()){
    if(Global_users[username].follows.find(follow) != Global_users[username].follows.end()){
      map<string,bool>::iterator itFollow;
      itFollow = Global_users[username].follows.find(follow);
      Global_users[username].follows.erase(itFollow);
      Global_users[username].following--;
      return 1;
    }else{
      Global_users[username].follows.insert(pair<string,bool>(follow,0));
      Global_users[username].following++;
      return 0;
    }
  }else{
    return 2;
  }
}



//vector de threads definicion, para hacer algo meter

vector <thread> threads;
string serialize(const string& str){
	int len = str.length();
	const string strlen((char*)&len, 4);
	return strlen + str;
}

string deserialize(const char* str){
	const int len = *(const int*)(str);
	return string(str + 4, len);
}



//funcion para imprimir el timeline cambiar los cout por contenido a enviar y asi enviamos eso
int enviar(const string& mensaje)
{
	int iResult, longitud;
	char* send_char;
	send_char = new char[mensaje.length() + 1];
	longitud=mensaje.length();
	for (int ii = 0; ii < mensaje.length(); ii++){ send_char[ii] = mensaje[ii]; }
		iResult = send(ClientSocket, send_char, longitud, 0);
		delete[] send_char;
		return iResult;
}

//global messages[ident].
void print_timeline(const string& user_name)
{
  map<unsigned int, Messages>::reverse_iterator rit;
  int count = 0, proba;
  string msg;
  char *cont=new char;
   char* indent= new char;
  int length=Global_messages.size();
  int iResult, iSendResult, doneInt;
	char recvbuf[DEFAULT_BUFLEN];
	char *temporal;
	int recvbuflen = DEFAULT_BUFLEN;
	string user, ACK;
  cout << endl << "Printing " << user_name << " timeline" << endl;
  iniciate_Mutex.lock();
  proba=enviar(serialize("mybaas"));
  iniciate_Mutex.unlock();
  for (rit=Global_messages.rbegin(); rit!=Global_messages.rend(); ++rit) {
    if (rit->second.user_name == user_name) {
		  itoa(rit->second.id,indent,10);
		  string iden=string(indent);
		  iniciate_Mutex.lock();
		  msg=serialize(indent );
		  proba=enviar(msg);
		  iniciate_Mutex.unlock();
		  iniciate_Mutex.lock();
		  msg=serialize( rit->second.content );
		  proba=enviar(msg);
		  iniciate_Mutex.unlock();
		  ++count;
	  }
	}
	itoa(count,cont,10);
	msg=serialize("fin");
	proba=enviar(msg);
  //cout << "----------------" << endl;

  //return msg;
}

string timeline(const string& username)
{
  map<unsigned int, Messages>::reverse_iterator rit;
  int count = 0;
  string msg, folo;
  char *cont=new char;
  char* indent= new char;
  int length=Global_messages.size();
  cout << endl << "Printing " << username << " Baas" << endl;
    for (rit=Global_messages.rbegin(); rit!=Global_messages.rend(); ++rit) {
		{
			folo=rit->second.user_name;

			if (Global_users[username].follows.find(folo) != Global_users[username].follows.end())
			{
				itoa(rit->second.id,indent,10);
				string iden=string(indent);
        msg=msg+ serialize(rit->second.user_name) + serialize(rit->second.content );
				++count;
			}
	  }
	}
	itoa(count,cont,10);
	msg=serialize(cont)+msg;
  //cout << "----------------" << endl;

  return msg;
}

bool log_out(string username){ // 0 OK, 1 error
  if(Global_users.find(username) != Global_users.end()){
      Global_users[username].loggedIn = 0;
      return 0;
    }else{
      return 1;
    }
}

int receive(SOCKET ClientSocket){ //return 0 = OK
	int iResult, iSendResult, doneInt;
	char recvbuf[DEFAULT_BUFLEN];
	char *temporal, *ACK_char;
	int recvbuflen = DEFAULT_BUFLEN;
	string user, type, data, ACK, doneStr,msg;
    Messages tempMessage;

	// Receive until the peer shuts down the connection
	do {
		iniciate_Mutex.lock();
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		iniciate_Mutex.unlock();
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
			    //LogIn a user
			    user = deserialize(temporal + 5);
			    cout << "Username: @" << user << " has logged in" << endl;
				iniciate_Mutex.lock();
			    doneInt = addLogged(user);
				iniciate_Mutex.unlock();
			    doneStr = to_string(doneInt);
        }else if(type == "1"){
          //Send new Baa
          tempMessage.user_name = deserialize(temporal + 5);
		  iniciate_Mutex.lock();
          cout << "Username: @" << tempMessage.user_name << " has Baaed" << endl;
          tempMessage.content = deserialize(temporal + 5 + tempMessage.user_name.length() + 4);
          tempMessage.id = last_id; ++last_id;
          tempMessage.timestamp = time(0);
          doneInt = newBaa(tempMessage);
		  iniciate_Mutex.unlock();
          doneStr = to_string(doneInt);
        }else if(type == "2"){
			//My baas
			user = deserialize(temporal + 5);
			print_timeline(user);     
			//si queremos más metemos un contador de 10 y opcion para meter más
        }else if(type == "3"){
			    //Unbaa
		  user = deserialize(temporal + 5);
		  //cout << "Username: " << user << " is removing a tweet";
		  iniciate_Mutex.lock();
          data = deserialize(temporal + 5 + user.length() + 4);
          //cout << data;
          doneInt = unBaa(user,data);
		  iniciate_Mutex.unlock();
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
			iniciate_Mutex.lock();
			data = deserialize(temporal + 5 + user.length() + 4);
			//cout << data;
			doneInt = Follow(user,data);
			iniciate_Mutex.lock();
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
		    //cout << "\n\nUsername: " << user;
			msg=timeline(user);
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
			//    Enviar ACK
			////////////////////////////////////////////////////////////////////////////////
			// Send an initial buffer: sendbuf contiene la frase que mandas
			if (type=="2"){
				
			}else if (type=="5"){
				ACK =serialize("timeline")+msg;
				iniciate_Mutex.lock();
				iResult=enviar(ACK);
				iniciate_Mutex.unlock();
			}else{
			  ACK = serialize("ACK") + serialize(doneStr);
			  iniciate_Mutex.lock();
			  iResult=enviar(ACK);
			  iniciate_Mutex.unlock();
			}
			
			//cout << "\nACK enviado!";
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
		}
		else if (iResult == 0) {
			printf("Closing connection...\n");
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


	while (true){
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		printf("New connection accepted\n");
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
	for(int ct=0;ct<threads.size();ct++)
	{
		if(threads[ct].joinable())
		{
			threads.erase(threads.begin()+ct-1);
		}
	}
	return 0;
}