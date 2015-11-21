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
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
    #include <windows.h>
    #include <winsock.h>
    #include <io.h>
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
      #include <sys/time.h>
   #endif
   #ifdef irix6
      #include <sys/poll.h>
   #endif
   #ifdef hp700
      #include <sys/poll.h>
      extern "C" int select(int, int*, int*, int*, const struct timeval*);
   #endif
#endif
#include "servers.h"

ServerConnection *ServerConnection::connections_list      = NULL;
ServerConnection *ServerConnection::connections_list_last = NULL;

ServerConnection::ServerConnection() {
	m_Service      = NULL;
	m_ConnectionId = -1;
	m_Socket = -1;
	m_Next = NULL;
}


ServerConnection* ServerConnection::GetNext(void) {
	return m_Next;
}


char* ServerConnection::GetService(void) {
	return m_Service;
}


int ServerConnection::Connect(char* service) {
int service_port;
int tcp_addr;
char full_service_name[1024];

    

    strcpy(full_service_name,"Dislite:");
	strcat(full_service_name,service);

    if(NameServerGetService(full_service_name,  service_port, tcp_addr)==1) {
        m_Server.sin_family      = AF_INET;
        m_Server.sin_port        = htons(service_port);
        m_Server.sin_addr.s_addr = htonl(tcp_addr);
		m_Service = strdup(service);
		m_Socket = socket (PF_INET, SOCK_STREAM, 0);
		if(m_Socket <0) return -1;
        if (connect(m_Socket, (struct sockaddr *) &m_Server, sizeof (m_Server)) < 0) return -2;
        int tmp_switch = 1;
        setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
		EvalCommand("login\tdish\thttpserver");
		return 1;
    }
	return 0;
}



ServerConnection* ServerConnection::AddNewConnection(char* service) {
ServerConnection* newConnection;

     newConnection = new ServerConnection();
	 if(newConnection->Connect(service)==1) {
        if(connections_list == NULL) {
	        connections_list      = newConnection;
	        connections_list_last = newConnection;
		} else {
            connections_list_last->m_Next = newConnection;
	        connections_list_last         = newConnection;
		}
		return newConnection;
	 }
	 return NULL;
}


ServerConnection* ServerConnection::FindConnection(char* service) {
    ServerConnection *cur = connections_list;
    while(cur != NULL){
	    if(strcmp(cur->GetService(), service) == 0)
	        return cur;
	    cur = cur->GetNext();
    }
    return NULL;
}

int ServerConnection::RemoveConnection(char* service) {
ServerConnection *p = connections_list;
ServerConnection *prev = NULL;

    while(p) {
       if(strcmp(p->GetService(), service) == 0) break;
	   prev = p;
	   p=p->GetNext();
	}

    if(p) { // If we 'v detect connection
		// Removing connection from the list
		if(prev) {
		    prev->m_Next = p->m_Next;
		} else {
			connections_list=p->m_Next;
		}
		if(connections_list_last==p) {
			if(prev) connections_list_last = prev;
			else     connections_list_last = connections_list;
		}
		delete p;
		return 1;
	}
	return 0;
}



ServerConnection* ServerConnection::EstablishConnection(char* service) {
ServerConnection* connection;

    connection = FindConnection(service);
	if(connection!=NULL) return connection;
    return AddNewConnection(service);

}


char* ServerConnection::EvalCommand(char* command) {
DISCommand *cmd;

    // New command will be send to the server
    cmd = new DISCommand(command,STDOUT_DISH_STREAM_CMD_TYPE);
    if(SendCommand(m_Socket, cmd)==0) {
		delete cmd;
		return NULL;
	}
    char *reply;
    unsigned char result_code;
    int success = ReceiveReply(m_Socket, &reply, result_code);
    delete cmd;
    if(success == 1) return reply;
    return NULL;
}



