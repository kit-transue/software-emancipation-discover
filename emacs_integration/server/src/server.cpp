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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#ifdef _WIN32 
	#include <io.h>
	#include <windows.h> 
	#include <winsock.h>
#else
   #ifndef hp10
	  #include <sysent.h>
   #endif
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

#include "lib.h"
#include "nameServCalls.h"
#include "CommProto.h"

#ifdef _WIN32
// On NT _eof returns true if empty or end of file reached.
// On Unix and Win95 return false on empty.
//    This causes read to hang.
#define EOFILE(strm) (_eof(fileno(strm)))
#else
// On Unix _eof returns true if empty or end of file reached.
// We add function which sets read to non blocking mode.
// On Win95 it currently still hangs.
#define EOFILE(strm) (feof(strm))
#endif

volatile int doDebug = 0;

void  ServerMessage( char *txt, char *typ){
#ifdef _DEBUG
#ifndef _WIN32
	FILE* log_fd = fopen("/usr/tmp/editorserver.log","a+");
#else
	FILE* log_fd = fopen("C:\\temp\\editorserver.log","a+");
#endif

	fprintf(log_fd,"%s: %s\n", typ, txt);
	fclose(log_fd);
#endif
}

//------------------------------------------------------------------------------------------------------
// This function will check if data available in the selected socket
//------------------------------------------------------------------------------------------------------

int WaitOnSocket(int socket) {
timeval timeout;
timeout.tv_sec  = 0;
timeout.tv_usec = 100000;
fd_set sock_set;

int    nfsd = 0;
#ifndef _WIN32
	nfsd = socket + 1;
#endif

    FD_ZERO(&sock_set);
    FD_SET(socket, &sock_set);

    if(select(nfsd,&sock_set,NULL,NULL,&timeout)>0)
        return 1;
    return 0;
}

void ConstructServicePrefix(char prefix[1024]) {
	char*	userName;
	char*	dnStart;
	// determine the current host name
	gethostname(prefix,1024);
	// this part of code just removes domain name suffix from the hostname
	if((dnStart = strchr(prefix,'.'))>0)
		prefix[dnStart-prefix] = 0;
	// constructing service prefix
	strcat(prefix,":");
	userName = getenv("USER");
#ifdef _WIN32
	if(userName == NULL) {
		char	userString[1024];
		DWORD	nUserSize = 1024;
		if(GetUserName(userString,&nUserSize)==TRUE)
			userName = userString;
	}
#endif
	if(userName!=NULL) {
		strcat(prefix,userName);
		strcat(prefix,":");
	}
}

char *GetIntegratorName(char * name) {
static char integratorName[1024];
	ConstructServicePrefix(integratorName);
	strcat(integratorName,name);
	return integratorName;
}

#ifndef _WIN32
int set_fl(int fd, int flags) {
	int val;
	if((val = fcntl(fd,F_GETFL, 0)) < 0)
		return -1;
	val |= flags;
	if(fcntl(fd,F_SETFL, val) < 0)
		return -1; 
	return 0;
}
#endif

int main (int argc, const char *argv[])           // Show state of the window
{        
  int index = 0,                      // Integer index
	  msz,                       // Return value of recv function
	  accept_new_connect;

  char *data;                // ASCII string 
  char integname[1050];
  char iname[1050];
  char szError[MAXMSGSZ];                 // Error message string
  char msgToEmacs[MAXLISPEXPR];
  char msgFromEmacs[MAXLISPEXPR];
  SOCKET WinSocket = INVALID_SOCKET,  // Window socket
         ClientSock = INVALID_SOCKET; // Socket for communicating 
                                      // between the server and client
  struct sockaddr_in local_sin;              // Local socket address
  struct sockaddr_in accept_sin;             // Receives the address of the 
                                      // connecting entity
  int accept_sin_len;                 // Length of accept_sin

  if(argc>1)
	for(int i=1;i<argc;i++) {
		if(strcmp(argv[i],"-debug")==0)	
			doDebug = 1;
	}
#ifdef _WIN32
  WSADATA WSAData;                    // Contains details of the Winsock
                                      // implementation
  // Initialize Winsock.
  if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) 
  {
    sprintf (szError, "WSAStartup failed.");
    ServerMessage (szError, "Server");
    return FALSE;
  }
#else
  set_fl(fileno(stdin),O_NONBLOCK);
#endif
  
  if(doDebug) {
  	printf("Ready to be debugged...");
  	while(doDebug){
  	}
  }

  // Create a TCP/IP socket, WinSocket.
  if ((WinSocket = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
  {
    sprintf (szError, "Allocating socket failed.");
    ServerMessage (szError, "Server");
    return FALSE;
  }

  // Fill out the local socket's address information.
  local_sin.sin_family = AF_INET;
  local_sin.sin_port = 0;  
  local_sin.sin_addr.s_addr = htonl (INADDR_ANY);

  // Associate the local address with WinSocket.
  if (bind (WinSocket, 
            (struct sockaddr *) &local_sin, 
            sizeof (local_sin)) == SOCKET_ERROR) 
  {
    sprintf (szError, "Binding socket failed.");
    ServerMessage (szError, "Server");
    closesocket (WinSocket);
    return FALSE;
  }

  if(argc > 1 && argv[1]) 
	strcpy(iname, argv[1]);
  else {
	sprintf (szError, "No integrator name specified on command line - e.g. editorServer EmacsIntegrator" );
	ServerMessage (szError, "Server");
	return FALSE;
  }

  // Forming the integrator service name
  strcpy(integname,GetIntegratorName(iname));

  // Establish a socket to listen for incoming connections.
  if (listen (WinSocket, MAX_PENDING_CONNECTS) == SOCKET_ERROR) 
  {
	sprintf (szError, "Listening to the client failed.");
	ServerMessage (szError, "Server");
	closesocket (WinSocket);
	return FALSE;
  }

  struct sockaddr_in  assigned;
  int len = sizeof(assigned);
  if(getsockname(WinSocket, (struct sockaddr *)&assigned, &len) == 0){
	if( NameServerRegisterService(integname, ntohs(assigned.sin_port)) == -1 ) {
	    sprintf (szError, "Error registering service (%s) on nameserver.",integname);
		ServerMessage (szError, "Server");
		closesocket (WinSocket);
		return FALSE;
	}
  } else {
    sprintf (szError, "Error registering service (%s) on nameserver.",integname);
	ServerMessage (szError, "Server");
	closesocket (WinSocket);
	return FALSE;
  }

  ServerMessage ("Registered emacs server successfully", "Server");
  
  int bExit = 0;
  while( !bExit ) {
	  accept_sin_len = sizeof (accept_sin);	  

	  // Accept an incoming connection attempt on WinSocket.
	  ClientSock = accept (WinSocket, 
						   (struct sockaddr *) &accept_sin, 
						   (int *) &accept_sin_len);

	  // Stop listening for connections from clients.
	  // closesocket (WinSocket);

	  if (ClientSock == INVALID_SOCKET) 
	  {
		sprintf (szError, "Accepting connection with client failed.");
		ServerMessage (szError, "Server");
		return FALSE;
	  }

	  for (accept_new_connect=0;!accept_new_connect;)
	  {
			// Wait for client to send command.  Must poll stdin on NT.  Pipes are not sockets.
			if (WaitOnSocket(ClientSock)) {
				// Receive Header from the client.
				data = getString(ClientSock);
				if( data != NULL ) {
					// Display the string received from the client.
					data += 2;
					sprintf (msgToEmacs, "%d:%s", strlen(data)+1, data);
					ServerMessage (msgToEmacs, "Server");
					if(!strcmp(data,"server_exit")) {
					   sendReply(ClientSock,"done");
					   bExit = 1;
					   break;
     				} else {
						sprintf (msgToEmacs, "%s\n", data);
						fwrite(msgToEmacs,1,strlen(msgToEmacs),stdout);
						fflush(stdout);
						sprintf(szError,"Message %s from client received",msgToEmacs);
						ServerMessage(szError,"Server");
         			}
				} else {
					// This means that the connection may have been closed.
					// Back out to accept new connection.
					accept_new_connect = 1;
					sprintf(szError,"NULL received");
					ServerMessage(szError,"Server");
				}
			}
			if(!accept_new_connect) {
				// reading reply
				while ( !EOFILE(stdin) ) {
					msz = read( fileno(stdin), msgFromEmacs, MAXLISPEXPR  );
					if (msz > 0) {
						msgFromEmacs[msz] = 0;
						if (sendReply (ClientSock, msgFromEmacs) != 1) 
						{
							sprintf (szError,"Sending data to the client failed.");
							ServerMessage (szError, "Server");
						}
						sprintf(szError,"Message %s from Emacs has been sent",msgFromEmacs);
						ServerMessage(szError,"Server");
					} else 
						break;
				}
			}
	  }
  } // while forever 

  NameServerUnRegisterService(integname);

  // Disable both sending and receiving on ClientSock.
  shutdown (ClientSock, 0x02);

  // Close ClientSock.
  closesocket (ClientSock);
#ifdef _WIN32
  WSACleanup ();
#endif

  return TRUE;
}











