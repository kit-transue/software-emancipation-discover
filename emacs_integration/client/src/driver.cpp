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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
	#include <windows.h>
	#include <winsock.h>
#else 
   #include <signal.h>
   #include <arpa/inet.h>
   #include <netinet/tcp.h>
   #include <netdb.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <unistd.h>
   #include <sys/types.h>
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

#ifndef _WIN32
#define FALSE	0
#define TRUE	1
#define INVALID_SOCKET	-1
#define closesocket(sock) close(sock)
#define BOOL	int
#endif

#include "CommProto.h"

#include "SocketComm.h"
#include "connect_mgr.h"
#include "editor_api.h"
#include "nameServCalls.h"
#include "startproc.h"
#include "debug.h"

extern CConnectionDescriptor Connection;

int do_debug = 0;
FILE*	log_fd;
char dbgFileName[1024];

static int nClients = 0;


// ------- Decls of local functions ------------
void RequestsLoop(void);
int ProcessClientQuery(ClientDescriptor* client);
void ProcessEditorEvent(CConnectionDescriptor* connection,ClientDescriptor* client);
static int do_shutdown = 0;
static int getModelLine(char* localfile, char* modelfile,int localline);

//--------------------------------------------------------------------------------------------
// This fuction will execute the mrg_diff to create diff file.
//---------------------------------------------------------------------------------------------
static BOOL make_diff(char* localfile, char* modelfile, char* diffile) {
  char* root;
  char* cmd;

  cmd = new char [5000];

  root = getenv("CODE_ROVER_HOME");
  if(root==NULL)
	root = getenv("PSETHOME");
  	
  if((root==NULL ) || (strlen(root)==0)) {
      strcpy(cmd,"C:/Discover/Bin/mrg_diff ");
  } else {
      strcpy(cmd,"\"");
      strcat(cmd,root);
      strcat(cmd,"\\bin\\mrg_diff\" ");
  }
  strcat(cmd,"\"");
  strcat(cmd,modelfile);
  strcat(cmd,"\"");
  strcat(cmd," ");
  strcat(cmd,"\"");
  strcat(cmd,localfile);
  strcat(cmd,"\"");
  strcat(cmd," ");
  strcat(cmd,"\"");
  strcat(cmd,diffile);
  strcat(cmd,"\"");

  DWORD retCode = start_new_process(cmd,START_AND_WAIT);

  delete cmd;
  return retCode!=-1;
}
//---------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------
// This fuction will execute the mrg_diff to create diff file.
//---------------------------------------------------------------------------------------------
static int map_lines(char* localfile, char* basefile, char* diffile, int localline) {
	char* root;
	char cmd[6000];

	_DBG(fprintf(log_fd,"Map_Lines ...\n"));
	root = getenv("CODE_ROVER_HOME");
	if(root==NULL)
		root = getenv("PSETHOME");
		
	if((root==NULL ) || (strlen(root)==0)) {
	  strcpy(cmd,"C:/Discover/Bin/mrg_update -mapL_to_L ");
	} else {
	  strcpy(cmd,"\"");
	  strcat(cmd,root);
	  strcat(cmd,"\\bin\\mrg_update\" -mapL_to_L ");
	}
	char num[20];
	sprintf(num,"%d",localline);
	strcat(cmd,num);
	strcat(cmd," ");
	strcat(cmd,"\"");
	strcat(cmd,basefile);
	strcat(cmd,"\"");
	strcat(cmd," ");
	strcat(cmd,"\"");
	strcat(cmd,diffile);
	strcat(cmd,"\"");
	strcat(cmd," ");
	strcat(cmd,"\"");
	strcat(cmd,localfile);
	strcat(cmd,"\"");
#ifdef _WIN32
	HANDLE ReadHandle;
	HANDLE WriteHandle;

	SECURITY_ATTRIBUTES sattr;
	memset(&sattr, 0, sizeof(sattr));
	sattr.nLength        = sizeof(sattr);
	sattr.bInheritHandle = TRUE;

	_DBG(fprintf(log_fd,"Creating pipe ..."));
	if(!CreatePipe(&ReadHandle, &WriteHandle, &sattr, 0)){
		_DBG(fprintf(log_fd,"FALIED!!!\n"));
		return -1;
	}
	_DBG(fprintf(log_fd,"OK.\n"));

	_DBG(fprintf(log_fd,"Starting  mrg_update ...\n"));
	DWORD dwDiffExitCode = start_new_process(cmd,START_AND_WAIT,
					GetStdHandle(STD_INPUT_HANDLE),
					WriteHandle,
					GetStdHandle(STD_ERROR_HANDLE));
	char lineStr[200];
	unsigned long readed;
	if(dwDiffExitCode==0) {
		ReadFile(ReadHandle,lineStr,200,&readed,NULL);
	} else {
		strcpy(lineStr,"-1\n%");
		readed = 4;
		_DBG(fprintf(log_fd,"Mrg_update FAILED!!!\n"));
	}

	lineStr[readed-2]=0;
	_DBG(fprintf(log_fd,"Map_Lines replies %s \n",lineStr));
	CloseHandle(ReadHandle);
	CloseHandle(WriteHandle);
	return atoi(lineStr);
#else
	int pfd[2];
	_DBG(fprintf(log_fd,"Creating pipe ..."));
	if(pipe(pfd)==-1) {
		_DBG(fprintf(log_fd,"FALIED!!!\n"));
		return -1;
	}
	_DBG(fprintf(log_fd,"OK.\n"));

	_DBG(fprintf(log_fd,"Starting  mrg_update ...\n"));
	DWORD dwDiffExitCode = start_new_process(cmd,START_AND_WAIT,
					-1,
					pfd[1],
					-1);
	char lineStr[200];
	unsigned long readed;
	if(dwDiffExitCode==0) {
		readed = read(pfd[0],lineStr,200);
	} else {
		strcpy(lineStr,"-1\n%");
		readed = 4;
		_DBG(fprintf(log_fd,"Mrg_update FAILED!!!\n"));
	}

	lineStr[readed-1]=0;
	_DBG(fprintf(log_fd,"Map_Lines replies %s \n",lineStr));
	close(pfd[0]);
	close(pfd[1]);
	return atoi(lineStr);
#endif
}
//---------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// This static function will use mrg_update to map line in the current file into line
// in the original file which was used during modelbuild.
//------------------------------------------------------------------------------------------
static int getModelLine(char* localfile, char* basefile,int localline) {
char* diffile;
	// This function will create diff file which we will need 
	// to map lines
    _DBG(fprintf(log_fd,"Line %d in file <%s> from local file <%s>.\n",localline,basefile,localfile));
    diffile = new char[strlen(basefile)+6];
	strcpy(diffile,basefile);
	strcat(diffile,".diff");
	if(make_diff(localfile,basefile,diffile)==FALSE) {
		 delete diffile;
		 return -1;
    }
	int res = map_lines(basefile,localfile,diffile,localline);
	delete diffile;
	return res;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// This static function will use mrg_update to map line in the original file into line
// in the modified file
//------------------------------------------------------------------------------------------
static int getPrivateLine(char* localfile, char* basefile,int localline) {
char* diffile;
	// This function will create diff file which we will need 
	// to map lines
    _DBG(fprintf(log_fd,"Line %d in local file <%s> from model file <%s>.\n",localline,localfile,basefile));
    diffile = new char[strlen(basefile)+6];
	strcpy(diffile,basefile);
	strcat(diffile,".diff");
	if(make_diff(localfile,basefile,diffile)==FALSE) {
		 delete diffile;
		 return -1;
	}
	int res = map_lines(localfile,basefile,diffile,localline);
	delete diffile;
	return res;
}
//------------------------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Main function will register the editor service
//---------------------------------------------------------------------------
int main(int argc, char **argv)  {
int len;
int i;
int	bNameServerStarted = 0;
	#ifdef _WIN32
		WORD    wVersionRequested;  
		WSADATA wsaData; 
		int     err; 
   
		wVersionRequested = MAKEWORD(1, 1); 
		err               = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0) {
			printf("no sockets available\n");
			return -1;
		}
		putenv("DIS_NLUNIXSTYLE=1");
	#endif 

	// check for debug
	for(i=1;i<argc;i++)
		if(strcmp(argv[i],"-debug")==0) {
			if(((i+1)<argc) && strlen(argv[i+1])>0) {
				do_debug = 1;
				strcpy(dbgFileName,argv[i+1]);
				_DBG_OPEN();
			}
		}
	//	_asm int 3;
	// Checking if we have an instance of this driver and if we
	// have name server running at all
    int res = NameServerGetService(GetEditorName(),
		                           Connection.m_DriverPort,
								   Connection.m_DriverAddress);
	switch(res) {
	   // No name server running
	   case -1 : 
				 if(LaunchNameServer(START_SERVER)==0) {
						_DBG(fprintf(log_fd,"Name server start failed.\n"));
						return 0;
				 }
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
                      _DBG(fprintf(log_fd,"Registering %s driver.\n",GetEditorName()));
                      NameServerRegisterService(GetEditorName(), ntohs(assigned.sin_port));
				 }
				 for(i=0;i<200;i++) {
				    if(NameServerGetService(GetEditorName(),Connection.m_DriverPort,Connection.m_DriverAddress)==1) break;
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

	// OK, here we have name server running and service registered.
	// We will try to connect to the MSDEV integrator

    _DBG(fprintf(log_fd,"Ready to process requests.\n"));
	RequestsLoop();
    _DBG(fprintf(log_fd,"Terminating driver.\n"));
	if(bNameServerStarted==1)
		LaunchNameServer(STOP_SERVER);
    closesocket(Connection.m_DriverListeningSocket);
	_DBG_CLOSE();
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

//--------------------------------------------------------------------------------------------
// This function will inspect clients list in order to determine which clients
// need to query server. The data will be forwarded from the client socket
// into model server socket and reply will be forwarded from model server socket
// to the client socket.
//--------------------------------------------------------------------------------------------
void ProcessClients(void) {
static ClientDescriptor* hang;
	int i=0;
    _DBG(fprintf(log_fd,"Trying to process client request.\n"));
    ClientDescriptor *cur = ClientDescriptor::sd_list;
    while(ClientDescriptor::sd_list!=NULL && cur != NULL) {
		if(CheckSocket(cur->m_ClientSocket)) {
			// Processing client request
	        _DBG(fprintf(log_fd,"Processing client request. %d\n",i++));
			if(ProcessClientQuery(cur)) break;
        }
        cur = cur->m_Next;
    }
}
//--------------------------------------------------------------------------------------------

extern int bIntegratorConnected;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void SendEvents(void) {
static ClientDescriptor* hang;
int		nimListeners = 0;

    ClientDescriptor *cur = ClientDescriptor::sd_list;
    while(ClientDescriptor::sd_list!=NULL && cur != NULL) {
	    // Sending event to the client
		ProcessEditorEvent(&Connection,cur);
        cur = cur->m_Next;
		nimListeners++;
    }

	if(nimListeners<=0) {
		bIntegratorConnected = 0;
		closesocket(Connection.m_IntegratorSocket);
		Connection.m_IntegratorSocket = INVALID_SOCKET;
	}
}
//-----------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
// Loop while not shutting down. Will listen clients socket and administrator socket,
// will process clients and administrator requests if any.
//-------------------------------------------------------------------------------------------
void RequestsLoop(void) {
int sockets[3000];

   struct sockaddr s;
   int s_size = sizeof(s);
   do {
	  int amount = MakeSocketsArray(Connection.m_DriverListeningSocket,Connection.m_IntegratorSocket,sockets);
      WaitSocket(sockets,amount); 
      if(CheckSocket(Connection.m_DriverListeningSocket)) {
          _DBG(fprintf(log_fd,"Accepting new client.\n"));
	      int cli_connection = accept(Connection.m_DriverListeningSocket, &s, &s_size);
          int tmp_switch = 1;
          setsockopt(cli_connection, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
	      if(cli_connection >= 0) {
	          _DBG(fprintf(log_fd,"Connecting new client.\n"));
	          ConnectClient(cli_connection, &s);
			  continue;
		  }
	  }
	  if(CheckSocket(Connection.m_IntegratorSocket)) {
	 	 // Processing editor event
          _DBG(fprintf(log_fd,"Sending Events.\n"));
		 SendEvents();
      }
      // Performing data translstion from the existing client sockets
	  // to the existing model sockets
      ProcessClients();
   } while(!do_shutdown);

   // tell emacs server to exit
   sendReply(Connection.m_IntegratorSocket,"server_exit");
   // wait for any reply
   ProcessEditorEvent(&Connection,NULL);
}
//--------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------
// This function will process given command with it's arguments and will send the reply
//---------------------------------------------------------------------------------------------
int ProcessOneCommand(ClientDescriptor* client, char args[10][1000]) {
	//register a new client
	if(strcmp(args[0],"register")==0) {
		nClients++;
		sendReply(client->m_ClientSocket,"done");
		ShowMessage("Done");
		_DBG(fprintf(log_fd,"Reply was send.\n"));
		return 1;
	}
	// unregister a client
	if(strcmp(args[0],"unregister")==0) {
		nClients--;
		if(nClients<1)
			do_shutdown = 1;
		sendReply(client->m_ClientSocket,"done");
		ShowMessage("Done");
		_DBG(fprintf(log_fd,"Reply was send.\n"));
		return 1;
	}
	// We will try to connect to the editor integrator
	if( bIntegratorConnected != 1) {
		// OK, here we have name server running and service registered.
		if(ConnectToIntegrator(Connection.m_IntegratorPort,Connection.m_IntegratorAddress)==0) {
			_DBG(fprintf(log_fd,"Unable to connect to the integrator server.\n"));
			sendReply(client->m_ClientSocket,"Error: Unable to connect to the integrator server.");
			return 0;
		}
		_DBG(fprintf(log_fd,"Connected to the integrator server.\n"));
	}
	// Open file using given line number
	if(strcmp(args[0],"open")==0) {
		OpenFileOnLine(args[1],atoi(args[2]));
		sendReply(client->m_ClientSocket,"done");
		ShowMessage("Done");
		_DBG(fprintf(log_fd,"Reply was send.\n"));
		return 1;
	}
	// Selects the token on the given line
	if(strcmp(args[0],"mark")==0) {
		MakeSelectionOnLine(args[1],atoi(args[2]),atoi(args[3]),atoi(args[4]));
		sendReply(client->m_ClientSocket,"done");
		_DBG(fprintf(log_fd,"Reply was send.\n"));
		return 1;
	}
	// Open file using given offset (not implemented under DeveloperStudio)
	if(strcmp(args[0],"openoffset")==0) {
		sendReply(client->m_ClientSocket,"not implemented");
		_DBG(fprintf(log_fd,"Reply was send.\n"));
		return 0;
	}
	// Selects the token using the start and end offsets (not implemented under DeveloperStudio)
	if(strcmp(args[0],"markoffset")==0) {
		sendReply(client->m_ClientSocket,"not implemented");
		_DBG(fprintf(log_fd,"Reply was send.\n"));
		return 0;
	}
	// Shows string from the client in the editor status window
	if(strcmp(args[0],"info")==0) {
		ShowMessage(args[1]);
        sendReply(client->m_ClientSocket,"done");
		_DBG(fprintf(log_fd,"Reply was send.\n"));
		return 1;
	}
	// Saves current editor selection in the buffer. We will query selection parameters
	// later and they must be unchanged.
	if(strcmp(args[0],"freeze")==0) {
	    FreezeSelection();
		sendReply(client->m_ClientSocket,"done");
		_DBG(fprintf(log_fd,"Reply was send.\n"));
		return 1;
	}

	// Returns the name of the file on which we freeze our selection.
	if(strcmp(args[0],"file")==0) {
		char* file = GetSelectionFile();
        sendReply(client->m_ClientSocket,file);
		_DBG(fprintf(log_fd,"Reply was send.\n"));
		return 1;
	}

	// Returns the selected line. Must not be implemented under Emacs
	if(strcmp(args[0],"line")==0) {
	   int line = GetSelectionLine();
	   char lineStr[20];
	   sprintf(lineStr,"%d",line);
       sendReply(client->m_ClientSocket,lineStr);
	   _DBG(fprintf(log_fd,"Reply was send.\n"));
	   return 1;
	}

	// Returns the offset to the selection. Do not exist under DeveloperStudio
	if(strcmp(args[0],"offset")==0) {
       sendReply(client->m_ClientSocket,"not implemented");
	   _DBG(fprintf(log_fd,"Reply was send.\n"));
	   return 1;
	}

	// Returns the offset in the selected line. Must not be implemented under Emacs
	if(strcmp(args[0],"lineoffset")==0) {
	   int column = GetSelectionLineOffset();
	   char columnStr[20];
	   sprintf(columnStr,"%d",column);
       sendReply(client->m_ClientSocket,columnStr);
	   _DBG(fprintf(log_fd,"Reply was send.\n"));
	   return 1;
	}


	// Converts given model file line into modified file line
	if(strcmp(args[0],"model_line_to_private_line")==0) {
	   int line = getPrivateLine(args[1],args[2],atoi(args[3]));
	   char lineStr[20];
	   sprintf(lineStr,"%d",line);
       sendReply(client->m_ClientSocket,lineStr);
	   _DBG(fprintf(log_fd,"model_line_to_private_line : %d -> %s \n", line, lineStr));
	   return 1;
	}

	// Converts given model file line into modified file line
	if(strcmp(args[0],"private_line_to_model_line")==0) {
	   int line = getModelLine(args[1],args[2],atoi(args[3]));
	   char lineStr[20];
	   sprintf(lineStr,"%d",line);
       sendReply(client->m_ClientSocket,lineStr);
	   _DBG(fprintf(log_fd,"private_line_to_model_line : %d -> %s \n", line, lineStr));
	   return 1;
	}


	// Converts given model file line into modified file line
	if(strcmp(args[0],"close_driver")==0) {
       sendReply(client->m_ClientSocket,"driver closed");
	   do_shutdown = 1;
	   return 1;
	}

    // We do not know this command 
    sendReply(client->m_ClientSocket,"unknown command");
    _DBG(fprintf(log_fd,"Unknown command %s.\n",args[0]));
	return 0;
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
	command = getString(client->m_ClientSocket);
    if(command == NULL) {
         _DBG(fprintf(log_fd,"Client disconnected. Removing dead client.\n"));
	     ClientDescriptor::RemoveClient(client);
		 return 1;
	}
	readed = strlen(command);
    _DBG(fprintf(log_fd,"Driver: Processing client query. (%s)\n", command));

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
    if(chcount>0) {
		word[chcount] = 0;
		strcpy(vals[lcount++],word);
	}

	if(lcount>0) {
		return ProcessOneCommand(client,vals);
	}
	return 0;
}
//--------------------------------------------------------------------------------------------

void ProcessEditorEvent(CConnectionDescriptor* connection,ClientDescriptor* client) {

	char* cmd = NULL;

	if(bIntegratorConnected == 1) {
		cmd = new char[getCommandLength()];
		int size = getDataFromServer(connection->m_IntegratorSocket, cmd, getCommandLength(), EditorEventHdr);
		_DBG(fprintf(log_fd,"Event block size %d.\n",size));
		if(size>0) {
			_DBG(fprintf(log_fd,"Editor event %s\n",cmd));
			if(client!=NULL)
				sendEvent(client->m_ClientSocket,cmd);
		} else {
			bIntegratorConnected = 0;
			connection->m_IntegratorSocket = INVALID_SOCKET;
		}
		if(cmd != NULL)
			delete cmd;
	}
}
//--------------------------------------------------------------------------------------------

