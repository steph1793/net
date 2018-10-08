#include <cstdlib>
#include <csignal>
#include "client.h"
#include <ctime>
#include <ctype.h>


Client *c = NULL;

void signal_handler(int signum){
        c->Clean();
        exit(signum);
}

int main(int argc,char** argv){
	
	if( argc<2) {
		cout << "check arguments : must add client id (1 or 2 for this test)\n";
		return -1;
	}
	else{
		try{
			int a = stoi(argv[1]);
		}catch(exception&e) {cout << "arguments must be a digit (1 or 2)\n"; return -1;}
	}

        string s = "";
        signal(SIGINT, signal_handler);
        c = new Client("localhost");
     	clock_t start;
	double duration;


        if(!c->Init()) return -1; //we initialize the client


	start = clock();
        while(!c->shutdown){
                duration = (clock() -start)/(double) CLOCKS_PER_SEC;
                if(duration >= 2){ //we send messages every 2 sec
                	if(atoi(argv[1])==2){ c->Send("helloooo");} //we suppose we have 2 clients
			else c->Send("haha");
			start = clock();
		}
                s = c->GetResponse();
                if(!s.empty()) cout << s << endl;
        }
	return 0;
}

