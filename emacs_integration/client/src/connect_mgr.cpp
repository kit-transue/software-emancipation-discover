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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifdef _WIN32
    #include <windows.h>
    #include <winsock.h>
    #include <io.h>
#define WaitASecond()	Sleep(1000)
#else
#define WaitASecond()	sleep(1)
   #ifndef hp10
   	#include <sysent.h>
   #endif
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
   #define closesocket(sock) close(sock)
   #define INVALID_SOCKET 	-1
#endif

#include "SocketComm.h"
#include "connect_mgr.h"
#include "nameServCalls.h"
#include "startproc.h"
#include "debug.h"

#include "editor_api.h"

char signature[]={27,0,0,27};

CConnectionDescriptor Connection;
ClientDescriptor *ClientDescriptor::sd_list      = NULL;
ClientDescriptor *ClientDescriptor::sd_list_last = NULL;

//------------------------------------------------------------------------------------------------------

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
// This function will create new client record in the clients linked list.
//--------------------------------------------------------------------------------------------
void CreateNewClient(int id, int client_socket, int client_port, int client_tcp) {

     ClientDescriptor* descr;
	 descr = new ClientDescriptor(id,client_socket,client_port,client_tcp);
	 descr->AddNewClient(descr);

}
//--------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
// This procedure will create a listen socket for client requests channel
//--------------------------------------------------------------------------------------------
int CreateListener(void) {
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

int	bIntegratorConnected = 0;
//---------------------------------------------------------------------------
// This static function will try to connect to the integrator server and
// if impossible, it will try to launch it.
//---------------------------------------------------------------------------
int ConnectToIntegrator(int& integrator_port, int& integratoraddr) {
	int		i;
	char	full[1050];
	int		IntegratorCreator = 0;

	bIntegratorConnected = 0;
        // Forming the integrator service name
        strcpy(full,GetIntegratorName());

	switch(NameServerGetService(full,Connection.m_IntegratorPort,
									Connection.m_IntegratorAddress)) {
		// We'v got it!
		case 1 :  break;
			// No nameserver detected
		case -1:  
			if(LaunchNameServer(START_SERVER)==0) 
				return 0;
			// Nameserver OK but we are unable to find integration service
			// Fall through if nameserver launch successful.
 		case 0 :  
			if(LaunchIntegrator()==0) 
				return 0;				          
			IntegratorCreator = 1;
			// Trying to connect to the integrator again - timeout 
			// to give it a chance to register it's service.
			for(i=0;i<200;i++) {
			    if(NameServerGetService(full,Connection.m_IntegratorPort,
								             Connection.m_IntegratorAddress)==1) 
					break;
			}
			// Timeout expied - we were unable to connect to
			// the integrator
			if(i==200) 
				return 0;
			break;
	}
	_DBG(fprintf(log_fd,"Connecting to the integrator..."));
	Connection.m_IntegratorSocket = ConnectToServer(Connection.m_IntegratorPort,Connection.m_IntegratorAddress);
	if(Connection.m_IntegratorSocket<1) {
		_DBG(fprintf(log_fd,"Attempt failed... RetCode=%d\n",Connection.m_IntegratorSocket));
		return 0;
	} else {
		// We need to decrease reference count in the case we are starting
		// integrator from this application copy.
		_DBG(fprintf(log_fd,"OK\n"));
		if(IntegratorCreator==1)  decrementRefCount(Connection.m_IntegratorSocket);
		bIntegratorConnected = 1;
	}

	return 1;
}
//---------------------------------------------------------------------------

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

    if(socket<0) return 0;
 
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

   if(amount == 0) {
	WaitASecond();
	return 0;
   }

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
int MakeSocketsArray(int server, int integrator, int *sockets) {
static ClientDescriptor* hang;

    int amount = 0;
	if(server != INVALID_SOCKET) {
		sockets[amount++]=server;
	}
	if(integrator != INVALID_SOCKET)
		sockets[amount++]=integrator;
    ClientDescriptor *cur = ClientDescriptor::sd_list;
    while(ClientDescriptor::sd_list!=NULL && cur != NULL) {
		if(cur->m_ClientSocket!=INVALID_SOCKET)
			sockets[amount++]=cur->m_ClientSocket;
        cur = cur->m_Next;
    }
	return amount;
}
//--------------------------------------------------------------------------------------------

