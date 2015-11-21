/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
// StartExe.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "nameServCalls.h"
#ifdef _WIN32
    #include <windows.h>
    #include <winsock.h>
    #include <ws2tcpip.h> // for socklen_t
    #include <io.h>
#else
   #include <arpa/inet.h>
   #include <netinet/tcp.h>
   #include <netdb.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <unistd.h>
   #include <sys/types.h>
   #include <sys/uio.h>
   #include <sys/wait.h>
   #ifdef sun4
      #include <poll.h>
      #include <time.h>
      #include <sys/time.h>
   #endif
   #ifdef irix6
      #include <sys/poll.h>
   #endif
   #ifdef hp700
      #include <sys/poll.h>
      #include <time.h>
      extern "C" int select(int, int*, int*, int*, const struct timeval*);
   #endif
#endif

#include "debug.h"
#include "startproc.h"


static int nClients = 0;

#define REPLY_DELIM			   "\t"

#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif

typedef struct {
	int m_DriverAddress;
	int m_DriverPort;
	int m_DriverListeningSocket;
} CConnectionDescriptor;
static CConnectionDescriptor Connection;
static int do_shutdown = 0;


//############################################################################################
// This class represents the client record in the clients linked list.
// Any client record contain client name, client TCP address and port and server TCP 
// address and port.
//############################################################################################
class ClientDescriptor {
public:
    ClientDescriptor(int id, 
		             int client_socket,
		             int client_port,
		             int client_tcp_addr);

    ~ClientDescriptor();
    static void             AddNewClient(ClientDescriptor *sd);
    static void             RemoveClient(int id);
    static void             RemoveClient(ClientDescriptor *p);
    static void             RemoveAllClients(void);
    static ClientDescriptor *LookupID(int id);
    int   m_ID;
    int   m_ClientSocket;
    int   m_ClientPort;
    int   m_ClientTCP;
    ClientDescriptor *m_Next;
    static ClientDescriptor *sd_list;
    static ClientDescriptor *sd_list_last;
};

ClientDescriptor *ClientDescriptor::sd_list      = NULL;
ClientDescriptor *ClientDescriptor::sd_list_last = NULL;
int ProcessClientQuery(ClientDescriptor* client);
void RequestsLoop(void);


//------------------------------------------------------------------------------------------------------
// Constructor will create new client with the client name
//------------------------------------------------------------------------------------------------------
ClientDescriptor::ClientDescriptor(int id, 
								   int client_socket,
								   int client_port, 
								   int client_addr) 
{
	this->m_ID             = id;
    this->m_ClientSocket   = client_socket;
    this->m_ClientPort     = client_port;
    this->m_ClientTCP      = client_addr;
    this->m_Next           = NULL;

}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// Destructor will free the space allocated for the new client name
//------------------------------------------------------------------------------------------------------
ClientDescriptor::~ClientDescriptor() {
}
//------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------
// This method will add new client to the clients linked list
//------------------------------------------------------------------------------------------------------
void ClientDescriptor::AddNewClient(ClientDescriptor *sd) {
    if(sd_list == NULL){
	   sd_list      = sd;
	   sd_list_last = sd;
    } else {
	   sd_list_last->m_Next = sd;
	   sd_list_last       = sd;
    }
}
//------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------
// This function will return a pointer to the client record in the clients linked list
// or NULL of no client with the given name found.
//------------------------------------------------------------------------------------------------------
ClientDescriptor *ClientDescriptor::LookupID(int id)
{
    ClientDescriptor *cur = sd_list;
    while(cur != NULL){
	    if(cur->m_ID==id) return cur;
	    cur = cur->m_Next;
    }
    return NULL;
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This function will remove the client with the given name.
//------------------------------------------------------------------------------------------------------
void ClientDescriptor::RemoveClient(int id) {
    ClientDescriptor *prev = NULL;
    ClientDescriptor *cur  = sd_list;
    while(cur != NULL) {
	    if(cur->m_ID == id) {
			closesocket(cur->m_ClientSocket);
	        if(prev == NULL)
		        sd_list = cur->m_Next;
	        else
		        prev->m_Next = cur->m_Next;
	        if(sd_list_last == cur)
		    sd_list_last = prev;
	        delete cur;
	        return;
		}
	    prev = cur;
	    cur  = cur->m_Next;
    }
}
//------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------
// This function will remove client with the given address
//------------------------------------------------------------------------------------------------------
void ClientDescriptor::RemoveClient(ClientDescriptor *p) {
    ClientDescriptor *prev = NULL;
    ClientDescriptor *cur  = sd_list;
    while(cur != NULL) {
	    if(p==cur) {
		    closesocket(cur->m_ClientSocket);
	        if(prev == NULL)
		        sd_list = cur->m_Next;
	        else
		        prev->m_Next = cur->m_Next;
	        if(sd_list_last == cur)
		    sd_list_last = prev;
	        delete cur;
	        return;
		}
	    prev = cur;
	    cur  = cur->m_Next;
    }
}
//------------------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------------------
// This function will remove client with the given address
//------------------------------------------------------------------------------------------------------
void ClientDescriptor::RemoveAllClients(void) {
    ClientDescriptor *p;
    ClientDescriptor *cur  = sd_list;
    while(cur != NULL) {
		p=cur;
	    cur  = cur->m_Next;
		closesocket(p->m_ClientSocket);
		delete p;
    }
	sd_list=sd_list_last=NULL;

}
//------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
// This procedure will create a listen socket for client requests channel
//--------------------------------------------------------------------------------------------
int CreateListener() {
    int sock;
    struct sockaddr_in name;
    /* Create the socket. */
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	return -1;
    unsigned int set_option = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&set_option, sizeof(set_option));

    /* Give the socket a name. */
    name.sin_family      = AF_INET;
    name.sin_port        = 0;
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	    return -1;
    if(listen(sock, 5) < 0)
	   return -1;
	return sock;

}
//--------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
// This function will create new client record in the clients linked list.
//--------------------------------------------------------------------------------------------
void CreateNewClient(int id, int client_socket, int client_port, int client_tcp) {

     ClientDescriptor* descr;
	 descr = new ClientDescriptor(id,client_socket,client_port,client_tcp);
	 descr->AddNewClient(descr);

}
//--------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Main function will register the editor service
//---------------------------------------------------------------------------
int startExeMain(void)  {
	socklen_t	len;
	int		i;
	int		bNameServerStarted = 0;
	char	serviceName[1024];
	char*	currentUser;
	char*	dnStart;

	gethostname(serviceName,1024);
	if((dnStart = strchr(serviceName,'.'))>0)
		serviceName[dnStart-serviceName]=0;
	strcat(serviceName,":");
	currentUser = getenv("USER");
	if(currentUser!=NULL) {
		strcat(serviceName,currentUser);
		strcat(serviceName,":");
	}
	strcat(serviceName,"STARTEXE");
	_DBG(fprintf(log_fd,"Name of the service is : %s\n",serviceName));

	// Checking if we have an instance of this driver and if we
	// have name server running at all
    int res = NameServerGetService(serviceName,
		                           Connection.m_DriverPort,
								   Connection.m_DriverAddress);
	switch(res) {
	   // No name server running
	   case -1 : if(LaunchNameServer(START_SERVER)==0) return 0;
                 _DBG(fprintf(log_fd,"Name server started.\n"));
				 bNameServerStarted = 1;
	   // No instance of this driver
	   case 0  : // Any app which needs the editor service must request this socket.
	             // It will create separate communication channel in responce
	             Connection.m_DriverListeningSocket = CreateListener();
	             if(Connection.m_DriverListeningSocket < 0) {
	                 _DBG(fprintf(log_fd,"Unable to create clients requests socket.\n"));
	                 return 1;
				 }
                 _DBG(fprintf(log_fd,"Listening socket created.\n"));
                 // This block will register MSDEV editor driver in our name server
                 struct sockaddr_in assigned;
                 len = sizeof(assigned);
                 if(getsockname(Connection.m_DriverListeningSocket, (struct sockaddr *)&assigned, &len) == 0){
                      _DBG(fprintf(log_fd,"Registering StartEXE service.\n"));
                      NameServerRegisterService(serviceName, ntohs(assigned.sin_port));
				 }
				 for(i=0;i<200;i++) {
				    if(NameServerGetService(serviceName,Connection.m_DriverPort,Connection.m_DriverAddress)==1) break;
				 }
				 if(i==200) {
	                 _DBG(fprintf(log_fd,"Unable to register this service.\n"));
					 closesocket(Connection.m_DriverListeningSocket);
					 return 2;
				 }
				 break;
	   // This driver is already running
	   case 1  : 
				int first_part,second_part,third_part,fourth_part;
				first_part = Connection.m_DriverAddress/(256*256*256);
				Connection.m_DriverAddress -= 256*256*256*first_part;
				second_part = Connection.m_DriverAddress/(256*256);
				Connection.m_DriverAddress -= 256*256*second_part;
				third_part = Connection.m_DriverAddress/256;
				Connection.m_DriverAddress -= 256*third_part;
				fourth_part = Connection.m_DriverAddress;
				 _DBG(fprintf(log_fd,"This driver is already running.\nPort is %d TCP address is %d.%d.%d.%d\nAborting launch procedure.\n",Connection.m_DriverPort,first_part,second_part,third_part,fourth_part));
		         return 3;
	}

    _DBG(fprintf(log_fd,"Ready to process requests.\n"));
	RequestsLoop();
	if(bNameServerStarted == 1) {
	    _DBG(fprintf(log_fd,"Closing nameserver.\n"));
		LaunchNameServer(STOP_SERVER);
	} else {
	    _DBG(fprintf(log_fd,"Unregistering service %s.\n",serviceName));
		NameServerUnRegisterService(serviceName);
	}
    _DBG(fprintf(log_fd,"Terminating driver.\n"));
    closesocket(Connection.m_DriverListeningSocket);
    return 0;
}


//--------------------------------------------------------------------------------------------
// This function will run every time the new client try to connect to the editor
// It will create socket connection between the client and the currently 
// running pmod server and will start client log.
//--------------------------------------------------------------------------------------------
void ConnectClient(int client_socket, sockaddr *addr) {
static int ID_Counter = 0;
	int client_port = ((sockaddr_in *)addr)->sin_port;
	int client_tcp  = ((sockaddr_in *)addr)->sin_addr.s_addr;

    // We will not use password right now, but we may in future
	CreateNewClient(ID_Counter++,
		            client_socket,
					client_port,
					client_tcp);
    _DBG(fprintf(log_fd,"New client connected.\n"));
}
//--------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This function will check if data available in the selected socket
//------------------------------------------------------------------------------------------------------
int CheckSocket(int socket) {
timeval timeout;
timeout.tv_sec  = 0;
timeout.tv_usec = 0;
fd_set sock_set;
int    nfsd = 0;
#ifndef _WIN32
nfsd = FD_SETSIZE;
#endif

    FD_ZERO(&sock_set);
    FD_SET(socket, &sock_set);
#ifdef hp700
    if(select(nfsd,(int *)&sock_set,NULL,NULL,&timeout)>0) {
#else
    if(select(nfsd,&sock_set,NULL,NULL,&timeout)>0) {
#endif
        return 1;
	}
	return 0;
}
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// This function will check if data available in the selected socket
//------------------------------------------------------------------------------------------------------
int WaitSocket(int *sockets, int amount) {
timeval timeout;
timeout.tv_sec  = 8;
timeout.tv_usec = 0;
fd_set sock_set;
int    nfsd = 0;
#ifndef _WIN32
nfsd = FD_SETSIZE;
#endif

    FD_ZERO(&sock_set);
	for(int i=0;i<amount;i++) {
        FD_SET(*(sockets+i), &sock_set);
	}
#ifdef hp700
    if(select(nfsd,(int *)&sock_set,NULL,NULL,&timeout)>0) {
#else
    if(select(nfsd,&sock_set,NULL,NULL,&timeout)>0) {
#endif
        return 1;
	}
	return 0;
}
//------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------
// This function will inspect clients list in order to determine which clients
// need to query server. The data will be forwarded from the client socket
// into model server socket and reply will be forwarded from model server socket
// to the client socket.
//--------------------------------------------------------------------------------------------
int MakeSocketsArray(int server, int *sockets) {
static ClientDescriptor* hang;

    sockets[0]=server;
	int amount=1;
    ClientDescriptor *cur = ClientDescriptor::sd_list;
    while(ClientDescriptor::sd_list!=NULL && cur != NULL) {
		sockets[amount++]=cur->m_ClientSocket;
        cur = cur->m_Next;
    }
	return amount;
}
//--------------------------------------------------------------------------------------------




//--------------------------------------------------------------------------------------------
// This function will inspect clients list in order to determine which clients
// need to query server. The data will be forwarded from the client socket
// into model server socket and reply will be forwarded from model server socket
// to the client socket.
//--------------------------------------------------------------------------------------------
void ProcessClients(void) {
static ClientDescriptor* hang;

	_DBG(fprintf(log_fd,"Trying to process client request.\n"));
    ClientDescriptor *cur = ClientDescriptor::sd_list;
    while(ClientDescriptor::sd_list!=NULL && cur != NULL) {
		if(CheckSocket(cur->m_ClientSocket)) {
			// Processing client request
			if(ProcessClientQuery(cur)) break;
        }
        cur = cur->m_Next;
    }
}
//--------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
// Loop while not shutting down. Will listen clients socket and administrator socket,
// will process clients and administrator requests if any.
//-------------------------------------------------------------------------------------------
void RequestsLoop(void) {
int sockets[3000];

   struct sockaddr s;
   socklen_t s_size = sizeof(s);
   do {
	  int amount = MakeSocketsArray(Connection.m_DriverListeningSocket,sockets);
      WaitSocket(sockets,amount); 
      if(CheckSocket(Connection.m_DriverListeningSocket)) {
	      int cli_connection = accept(Connection.m_DriverListeningSocket, &s, &s_size);
          int tmp_switch = 1;
          setsockopt(cli_connection, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
	      if(cli_connection >= 0) {
	          _DBG(fprintf(log_fd,"Connecting new client.\n"));
	          ConnectClient(cli_connection, &s);
			  continue;
		  }
	  }
      // Performing data translstion from the existing client sockets
	  // to the existing model sockets
      ProcessClients();
   } while(!do_shutdown);
}
//--------------------------------------------------------------------------------------------

bool ProcessEnvVars(char* cmd,char* expandedCmd) {
	int			cmdLen = strlen(cmd);
	char*	substrStart = cmd;
	char*	firstDelim;
	char*	secondDelim = cmd;
	char	envVar[256];
	char*	pEnvVar;
	memset(envVar,0,256*sizeof(char));	
	do{
		firstDelim = strchr(secondDelim,'%');
		if(firstDelim!=NULL) {				
			secondDelim = strchr(firstDelim+1,'%');
			if(secondDelim==NULL)
				return false;
			strncat(expandedCmd,substrStart,firstDelim-substrStart);
			strncpy(envVar,firstDelim+1,secondDelim-firstDelim-1);
			pEnvVar = getenv(envVar);
			if(pEnvVar==NULL)
				return false;
			strcat(expandedCmd,pEnvVar);		
			secondDelim++;
			substrStart = secondDelim;
		}
	}while((firstDelim!=NULL) && (secondDelim<(cmd+cmdLen)));
	strncat(expandedCmd,substrStart,firstDelim-substrStart);
	return true;
}

#ifndef _WIN32
const int ERROR_SUCCESS = 0;
const int ERROR_ENVVAR_NOT_FOUND = -1;
static int lastError = 0;
void SetLastError(int errorCode) {
	lastError = errorCode;
}
int GetLastError() {
	return lastError;
}
char* FormatErrorMessage(int errorCode) {
	switch(errorCode) {
	case ERROR_SUCCESS:
		return (char *)"The operation completed successfully.";
	case ERROR_ENVVAR_NOT_FOUND:
		return (char *)"The system could not find the environment option that was entered.";
	default:
		return (char *)"The error code is unknown.";
	}
}
#endif

bool StartProcess(char* exeName, char* params, char* outputFile,DWORD *retCode, int waitType) {
	char	exeCmd[2048];
	
	memset(exeCmd,0,2048*sizeof(char));	
	if(ProcessEnvVars(exeName,exeCmd)==false) {
		SetLastError(ERROR_ENVVAR_NOT_FOUND);
		return false;
	}
	strcat(exeCmd," ");
	if(ProcessEnvVars(params,exeCmd+strlen(exeCmd))==false) {
		SetLastError(ERROR_ENVVAR_NOT_FOUND);
		return false;
	}
	if(outputFile == NULL)
		*retCode = start_new_process(exeCmd,waitType);
	else {
		*retCode = start_new_process(exeCmd,outputFile);
	}
	return (*retCode)!=-1;

}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------
// This function will process given command with it's arguments and will send the reply
//---------------------------------------------------------------------------------------------
int ProcessOneCommand(ClientDescriptor* client, char args[10][1000],int paramsCount) {
	int		cmdType	=	START_NO_WAIT;
	DWORD	exitCode;
	char*	outputFile = NULL;
	char	replyString[1024];

	if(strcmp(args[0],"register")==0) {
		nClients++;
		SendString(client->m_ClientSocket, "done");
		_DBG(fprintf(log_fd,"A new client has been registered.\n"));
		return 1;
	}
	if(strcmp(args[0],"unregister")==0) {
		nClients--;
		if(nClients<1){
			do_shutdown = 1;
		}
		SendString(client->m_ClientSocket, "done");
		_DBG(fprintf(log_fd,"A client has been unregistered.\n"));
		return 1;
	}
	// Starts detached process
	if(strcmp(args[0],"start")==0) {
		cmdType	=	START_NO_WAIT;
		_DBG(fprintf(log_fd,"Processing START_NO_WAIT command...\n"));
	} else {
		if(strcmp(args[0],"start_and_wait")==0) {
			cmdType	=	START_AND_WAIT;
			_DBG(fprintf(log_fd,"Processing START_AND_WAIT command...\n"));
		} else {
			if(strcmp(args[0],"start_wait_and_get_output")==0) {
				cmdType	= START_AND_WAIT;
				outputFile = args[paramsCount-1];
				_DBG(fprintf(log_fd,"Processing START_WAIT_AND_GET_OUTPUT command...\n"));
			} else {
				// We do not know this command 
				sprintf(replyString,"Error%s0%sunknown command",REPLY_DELIM,REPLY_DELIM);
				SendString(client->m_ClientSocket,replyString);
				_DBG(fprintf(log_fd,"Unknown command %s.\n",args[0]));
				return 0;
			}
		}
	}
	if(StartProcess(args[1],args[2],outputFile,&exitCode,cmdType)==true) {
		sprintf(replyString,"Done%s%d%s",REPLY_DELIM,exitCode,REPLY_DELIM);
		_DBG(fprintf(log_fd,"Command successed. Return code %d\n",exitCode));
	} else {
#ifdef _WIN32
		LPVOID lpMsgBuf;
		FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);
		// Save the error string.
		sprintf(replyString,"Error%s%d%s%s",REPLY_DELIM,&exitCode,REPLY_DELIM,(LPTSTR)lpMsgBuf);
		// Free the buffer.
		LocalFree( lpMsgBuf );
#else
		sprintf(replyString,"Error%s%d%s%s",REPLY_DELIM,&exitCode,REPLY_DELIM,FormatErrorMessage(GetLastError()));
#endif
		_DBG(fprintf(log_fd,"Command FAILED. Return code %d. Error %s\n",exitCode,replyString));
	}

	SendString(client->m_ClientSocket,replyString);
	return 1;
}
//----------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
// Client send editor an operation request
//--------------------------------------------------------------------------------------------
int ProcessClientQuery(ClientDescriptor* client) {
char* command;
int readed;
char vals[10][1000];

	// Redirecting query from the client to the server
	command = ReceiveString(client->m_ClientSocket);
    if(command == NULL) {
         _DBG(fprintf(log_fd,"Client disconnected. Removing dead client.\n"));
	     ClientDescriptor::RemoveClient(client);
		 return 1;
	}
	readed = strlen(command);
    _DBG(fprintf(log_fd,"Processing client query.\n"));

	// Tokenizing string query and placing all tokens into the 
	// vals matrix
	char word[1000];
	word[0]=0;
	int chcount=0;
	int lcount = 0;
	int i;
	for(i=0;i<readed;i++) {
		if(command[i]=='\t') {
			word[chcount] = 0;
			strcpy(vals[lcount++],word);
			chcount = 0;
			continue;
		}
		if(chcount<998) word[chcount++]=command[i];
	}
    if(chcount>=0) {
		word[chcount] = 0;
		strcpy(vals[lcount++],word);
	}

	if(lcount>0) {
		 ProcessOneCommand(client,vals,lcount);
		 return 1;
	}
	return 0;
}
//--------------------------------------------------------------------------------------------
