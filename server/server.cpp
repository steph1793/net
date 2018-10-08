#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include <cstring>
#include <csignal>
#include <string>


using namespace std;

bool running = true;
bool connected = false;
TCPsocket client;
TCPsocket client2;
TCPsocket server;
SDLNet_SocketSet set;



const unsigned short PORT = 1234;
const unsigned short BUFFER_SIZE = 1000;
const unsigned short MAX_SOCKETS = 3;
const unsigned short MAX_CLIENTS = MAX_SOCKETS-1;

const string SERVER_NOT_FULL = "OK";
const string SERVER_FULL = "FULL";

IPaddress  serverIp;
TCPsocket serverSocket;
TCPsocket clientSocket[MAX_CLIENTS];
bool socketIsFree[MAX_CLIENTS];
SDLNet_SocketSet socketSet;

char buffer[BUFFER_SIZE];
int receivedByte_count = 0;
int clientcount =0;
bool shutdownServer = false;



bool Init(){
	if (SDLNet_Init() == -1){
		return false;
	}
	socketSet = SDLNet_AllocSocketSet(MAX_SOCKETS);
	if(!socketSet) return false;

	for (int i=0;i<MAX_CLIENTS;i++){
		clientSocket[i] = NULL;	
		socketIsFree[i] = true;
	}

	if(SDLNet_ResolveHost(&serverIp,NULL,PORT) <0) return false;

	serverSocket = SDLNet_TCP_Open(&serverIp);
	if(!serverSocket) return false;

	SDLNet_TCP_AddSocket(socketSet, serverSocket);
	cout << "Initialisation reussie\n";
	return true;

}

void loop(){

	int activeNum = SDLNet_CheckSockets(socketSet,0);
	
	if(SDLNet_SocketReady(serverSocket)){
		if(clientcount < MAX_CLIENTS){
			int freespot = -100;
			for (int i=0;i<MAX_CLIENTS;i++){
				if(socketIsFree[i]){
					socketIsFree[i] = false;
					freespot = i;
					break;
				}
			}
			clientSocket[freespot] = SDLNet_TCP_Accept(serverSocket);
			SDLNet_TCP_AddSocket(socketSet, clientSocket[freespot]);
			clientcount++;

			////// We send a message to the client to tell him he is connected
			memset(buffer, '\0', BUFFER_SIZE);
			strcpy(buffer, "connected");
			SDLNet_TCP_Send(clientSocket[freespot], (void*)buffer, BUFFER_SIZE+1);
		}
		else{
			TCPsocket temp = SDLNet_TCP_Accept(serverSocket);

			/////// We send a message to say the room is full
			memset(buffer, '\0', BUFFER_SIZE);
                        strcpy(buffer, "full");
                        SDLNet_TCP_Send(temp, (void*)buffer, BUFFER_SIZE+1);

			SDLNet_TCP_Close(temp);
		}
	}

	for (int i=0;i<MAX_CLIENTS;i++){
		if(SDLNet_SocketReady(clientSocket[i])){
			memset(buffer, '\0', BUFFER_SIZE);
			receivedByte_count = SDLNet_TCP_Recv(clientSocket[i], buffer, BUFFER_SIZE);
			if(receivedByte_count <=0){
				SDLNet_TCP_DelSocket(socketSet, clientSocket[i]);
				SDLNet_TCP_Close(clientSocket[i]);
				clientSocket[i] = NULL;
				socketIsFree[i] = true;
				clientcount --;
			}
			else{
				for(int j=0;j<MAX_CLIENTS;j++){
					int len = strlen(buffer)+1;
					if(len>1 && j!=i && socketIsFree[j] == false){
						//// send the message to other clients
						cout << "Retransmitting the message : "<< buffer<< endl;
						SDLNet_TCP_Send(clientSocket[j], (void*)buffer, strlen(buffer)+1);
					}
				}
			}
		}
	}
}


void Clean(){
	SDLNet_FreeSocketSet(socketSet);
	SDLNet_TCP_Close(serverSocket);
	SDLNet_Quit();
}

void signal_handler(int signum){ 
	Clean();
	cout << "\nshutting down the server\n";
	exit(signum); 
	
}
 

int main(int argc,char** argv)
{
	signal(SIGINT, signal_handler);
        if(!Init()){
		cout << "Initilization unsuccessful\n";
		return -1;
	}
        do
        {
		loop();
        }while(!shutdownServer);
	Clean();
}
