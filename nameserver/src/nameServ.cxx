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
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <sys/types.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#include <ws2tcpip.h> // for socklen_t
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <winmain.h>
#else
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <netinet/tcp.h>
   #include <arpa/inet.h>
   #include <netdb.h>
   #include <unistd.h>
#ifdef sun4
#include <poll.h>
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
#define closesocket(sock) close(sock)
#endif

#include "nameServCalls.h"

int const ns_port       = NS_PORT;
int const info_port     = BROADCAST_SERVER_PORT;

int do_shutdown = 0;
int do_stop     = 0;
int do_debug    = 0;
FILE* log_fd = stderr;

struct sockaddr_in *parent = NULL;

#define DBG(stmt) if(do_debug) { stmt; }

class ServerDescriptor {
public:
    ServerDescriptor(char *name, int port, int tcp_addr);
    ~ServerDescriptor();

    void Update(int port, int tcp_addr);

    static void             AddNewServer(ServerDescriptor *sd);
    static void             RemoveServer(char *name);
    static ServerDescriptor *LookupName(char *name);

    char *name;
    int  port;
    int  tcp_addr;
    
    ServerDescriptor *next;
    static ServerDescriptor *sd_list;
    static ServerDescriptor *sd_list_last;
};

ServerDescriptor *ServerDescriptor::sd_list      = NULL;
ServerDescriptor *ServerDescriptor::sd_list_last = NULL;

static char const zeroc = 0;

//------------------------------------------------------------------------------------------------------

ServerDescriptor::ServerDescriptor(char *n, int p, int addr)
{
    this->name     = strdup(n);
    this->port     = p;
    this->tcp_addr = addr;
    this->next     = NULL;
}

ServerDescriptor::~ServerDescriptor()
{
    free(name);
}

void ServerDescriptor::Update(int p, int addr)
{
    this->port     = p;
    this->tcp_addr = addr;
}

void ServerDescriptor::AddNewServer(ServerDescriptor *sd)
{
    if(sd_list == NULL){
	sd_list      = sd;
	sd_list_last = sd;
    } else {
	sd_list_last->next = sd;
	sd_list_last       = sd;
    }
    DBG(fprintf(stderr, "Adding server: %s, port %d\n", sd->name, sd->port); fflush(stderr));
}

ServerDescriptor *ServerDescriptor::LookupName(char *name)
{
    ServerDescriptor *cur = sd_list;
    while(cur != NULL){
	if(strcmp(cur->name, name) == 0)
	    return cur;
	cur = cur->next;
    }
    return NULL;
}

void ServerDescriptor::RemoveServer(char *name)
{
    ServerDescriptor *prev = NULL;
    ServerDescriptor *cur  = sd_list;
    while(cur != NULL){
	if(strcmp(cur->name, name) == 0) {
	    if(prev == NULL)
		sd_list = cur->next;
	    else
		prev->next = cur->next;
	    if(sd_list_last == cur)
		sd_list_last = prev;
	    delete cur;
	    return;
	}
	prev = cur;
	cur  = cur->next;
    }
}

//------------------------------------------------------------------------------------------------------
int ConnectToParent(void)
{
    int sock;

    /* Create the socket. */
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	return -1;
    /* Give the socket a name. */
    if(connect(sock,  (struct sockaddr *)parent, sizeof (sockaddr_in)) < 0)
	return -1;
    return sock;
}

int CreateListener(void)
{
    int sock;
    struct sockaddr_in name;

    /* Create the socket. */
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	return -1;
#if 0
    // Eliminated because on Windows we were getting multiple nameservers started,
    // when the correct nameserver was busy with several requests and due to 
    // Windows workstation's limitation to 5 in the listen queue, connections to
    // nameserver were refused.
    unsigned int set_option = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&set_option, sizeof(set_option));
#endif

    /* Give the socket a name. */
    name.sin_family      = AF_INET;
    name.sin_port        = htons (ns_port);
    name.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	return -1;
    if(listen(sock, 40) < 0)
	return -1;
    return sock;
}


int CreateDatagramListener(void) {
int srv_sock;
struct sockaddr_in        srv_addr;

    srv_sock=socket(PF_INET,SOCK_DGRAM,0);
    if (srv_sock<0){
	    return -1;
	}
    srv_addr.sin_family      = AF_INET;
    srv_addr.sin_port        = htons (info_port);
    srv_addr.sin_addr.s_addr = INADDR_ANY;   /* Allow the server to accept connections  over any interface */

    /* Bind remote server's address and port */
    if (bind(srv_sock,(struct sockaddr *)&srv_addr,sizeof(srv_addr))<0){  
		return -1;
    }
    return srv_sock;
}


int RegisterServerOnParent(int port, char *name)
{
    int psock = ConnectToParent();
    if(psock <= 0)
	return 0;
    if(!SendCommand(psock, REGISTER_SERVER))
	return 0;
    if(!SendInteger(psock, port)){
	closesocket(psock);
	return 0;
    }
    if(!SendInteger(psock, strlen(name))) {
	closesocket(psock);
	return 0;
    }
    if(!SendBuffer(psock, name, strlen(name))) {
	closesocket(psock);
	return 0;
    }
    closesocket(psock);
    return 1;
}

int DoRegisterServer(int socket, sockaddr *addr)
{
    int port;
    if (!ReceiveInteger(socket, &port)) {
        closesocket(socket);
        return 0;
    }
    char *name = ReceiveString(socket);
    if (name == NULL) {
        closesocket(socket);
        return 0;
    }
    if (name[0] == 0) {
	return -1;
    }
    int success = 1;
    if(parent != NULL)
	success = RegisterServerOnParent(port, name);
    else {
	struct sockaddr_in *in = (sockaddr_in *)addr;
	ServerDescriptor *sd   = ServerDescriptor::LookupName(name);
	if(sd == NULL) {
	    sd = new ServerDescriptor(name, port, ntohl(in->sin_addr.s_addr));
	    ServerDescriptor::AddNewServer(sd);
	} else {
	    sd->Update(port, ntohl(in->sin_addr.s_addr));
	}
    }
    DBG(fprintf(stderr, "Registered Service: %s, port %d\n", name, port); fflush(stderr));
    return success;
}

int UnregisterServerOnParent(char *name)
{
    int psock = ConnectToParent();
    if(psock <= 0)
	return 0;
    if(!SendCommand(psock, UNREGISTER_SERVER))
	return 0;
    if(!SendInteger(psock, strlen(name))) {
	closesocket(psock);
	return 0;
    }
    if(!SendBuffer(psock, name, strlen(name))) {
	closesocket(psock);
	return 0;
    }
    closesocket(psock);
    return 1;
}

int DoUnregisterServer(int socket)
{
    char *name = ReceiveString(socket);
    if (name == NULL) {
	closesocket(socket);
        return 0;
    }
    if (name[0] == 0) {
	return -1;
    }
    DBG(fprintf(stderr, "Unregistering Service %s\n", name); fflush(stderr));
    if(parent != NULL) {
	UnregisterServerOnParent(name);
    } else {
	ServerDescriptor *sd  = ServerDescriptor::LookupName(name);
	if(sd != NULL) {
	    ServerDescriptor::RemoveServer(name);
	    DBG(fprintf(stderr, "Removing server: %s\n", name); fflush(stderr));
	}
    }
    return 1;
}

int TryServer(ServerDescriptor *sd)
{
    int sock;
    struct sockaddr_in name;
    
    /* Create the socket. */
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	return 1;
    /* Give the socket a name. */
    name.sin_family      = AF_INET;
    name.sin_port        = htons(sd->port);
    name.sin_addr.s_addr = htonl(sd->tcp_addr);
    DBG(fprintf(stderr, "Trying server %s/port %d.\n", sd->name, sd->port); fflush(stderr));
    if(connect(sock,  (struct sockaddr *) &name, sizeof (name)) < 0)
	return 0;
    DBG(fprintf(stderr, "Server alive.\n"); fflush(stderr));    closesocket(sock);
    return 1;
}

int DoListOfServers(int socket)
{
    if(parent) {
	int psock = ConnectToParent();
	if(psock <= 0)
	    return 0;
	if(!SendCommand(psock, LIST_OF_SERVERS))
	    return 0;
	int len;
	while(ReceiveInteger(psock, &len) && len != 0) {
	    if(!SendInteger(socket, len)) {
		closesocket(psock);
		return 0;
	    }
	    char *buf = new char[len + 1];
	    if(!ReceiveBuffer(psock, buf, len)) {
		closesocket(psock);
		delete [] buf;
		return 0;
	    }
	    buf[len] = 0;
	    if(!SendBuffer(socket, buf, len)) {
		closesocket(psock);
		delete [] buf;
		return 0;
	    }
	    delete [] buf;
	}
	closesocket(psock);
	if(!SendString(socket, ""))
	    return 0;
    } else {
	ServerDescriptor *curr = ServerDescriptor::sd_list;
	while(curr != 0) {   
	    char *name = curr->name;
	    int success = TryServer(curr);
	    curr = curr->next;
	    if(success) {
	        if(!SendString(socket, name))
		    return 0;
	    } else {
		DBG(fprintf(stderr, "Removing dead server: %s\n", name); fflush(stderr));
		ServerDescriptor::RemoveServer(name);
	    }
	}
	if(!SendString(socket, ""))
	    return 0;
    }
    return 1;
}

int SendServerInfo(int socket, ServerDescriptor *sd)
{
    unsigned char const cmd = 1;

    if(!SendCommand(socket, cmd))
	return 0;
    if(!SendInteger(socket, sd->port))
	return 0;
    if(!SendInteger(socket, sd->tcp_addr))
	return 0;
    return 1;
}

ServerDescriptor *FindServerOnParent(char *name)
{
    unsigned char cmd;

    int psock = ConnectToParent();
    if(psock <= 0)
	return NULL;
    if(!SendCommand(psock, FIND_SERVER))
	return NULL;
    if(!SendString(psock, name)) {
	closesocket(psock);
	return NULL;
    }

    if(!ReceiveCommand(psock, &cmd) || cmd == 0) {
	closesocket(psock);
	return NULL;
    }

    int  port;
    int  tcp_addr;

    if(!ReceiveInteger(psock, &port)) {
	closesocket(psock);
	return NULL;
    }
    if(!ReceiveInteger(psock, &tcp_addr)) {
	closesocket(psock);
	return NULL;
    }
    ServerDescriptor *sd = new ServerDescriptor("", port, tcp_addr);
    closesocket(psock);
    return sd;
}

int DoFindServer(int socket)
{
    char *name = ReceiveString(socket);
    if (!name) {
        return 0;
    }
    if (name[0] == 0) {
        return -1;
    }
    if(parent) {
	ServerDescriptor *sd = FindServerOnParent(name);
	if(sd) {
	    int success = SendServerInfo(socket, sd);
	    delete sd;
	    return success;
	} else {
	    int res = send(socket, &zeroc, 1, 0);
	    return res;
	}
    } else {
	DBG(fprintf(stderr, "Looking for name : %s\n", name); fflush(stderr));
	ServerDescriptor *sd = ServerDescriptor::LookupName(name);
	if(sd) {
	    int success = TryServer(sd);
	    if(!success) {
		ServerDescriptor::RemoveServer(name);
		DBG(fprintf(stderr, "Removing dead server: %s\n", name); fflush(stderr));
		sd = NULL;
	    }
	}
	if(sd == NULL){
	    int res = send(socket, &zeroc, 1, 0);
	    return res;
	}
	int success = SendServerInfo(socket, sd);
	return success;
    }
}

void SendStopCommand(void)
{
    u_long addr;
    struct sockaddr_in self_addr;

    addr                = inet_addr("127.0.0.1");
    struct hostent *ent = gethostbyaddr((char *)&addr, sizeof (addr), AF_INET);
    if(ent == NULL)
	return;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port   = htons(ns_port);
    self_addr.sin_addr   = *(in_addr *)*ent->h_addr_list;
    /* Create the socket. */
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	return;
    /* Give the socket a name. */
    if(connect(sock,  (struct sockaddr *)&self_addr, sizeof (sockaddr_in)) >= 0)
	SendCommand(sock, STOP);
    closesocket(sock);
}

int AlreadyRunning() 
{
    u_long addr;
    struct sockaddr_in self_addr;
    
    addr                = inet_addr("127.0.0.1");
    struct hostent *ent = gethostbyaddr((char *)&addr, sizeof (addr), AF_INET);
    if(ent == NULL)
	return 0;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port   = htons(ns_port);
    self_addr.sin_addr   = *(in_addr *)*ent->h_addr_list;
    /* Create the socket. */
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	return 0;
    /* Give the socket a name. */
    int running = 1;
    if(connect(sock,  (struct sockaddr *)&self_addr, sizeof (sockaddr_in)) < 0)
	running = 0;
    closesocket(sock);
    return running;
}

void ProcessRequests(int socket, sockaddr *addr)
{
    unsigned char cmd;

    if (ReceiveCommand(socket, &cmd)) {
        switch (cmd) {
        case REGISTER_SERVER:
	    DoRegisterServer(socket, addr);
            break;
        case LIST_OF_SERVERS:
	    DoListOfServers(socket);
            break;
	case FIND_SERVER:
	    DoFindServer(socket);
            break;
        case UNREGISTER_SERVER:
	    DoUnregisterServer(socket);
            break;
	case STOP:
	    do_shutdown = 1;
            break;
        }
    }
    closesocket(socket);
}

void DoInfoProcessing(int socket) {
   timeval timeout;
   timeout.tv_sec  = 0;
   timeout.tv_usec = 100000;

   fd_set socket_set;
   int    nfsd = 0;

#ifndef _WIN32
   nfsd = FD_SETSIZE;
#endif
   FD_ZERO(&socket_set);
   FD_SET(socket, &socket_set);



#ifdef hp700
    if(select(nfsd,(int *)&socket_set,NULL,NULL,&timeout)<=0) return;
#else
    if(select(nfsd,&socket_set,NULL,NULL,&timeout)<=0) return;
#endif
   char buf[1024];
   struct sockaddr from;
   socklen_t len=sizeof(from);
   buf[0]=0;
   int bytes;
   bytes=recvfrom(socket,buf,1024,0,&from,&len);
   DBG(fprintf(stderr, "Query received : %s\n",buf); fflush(stderr));
   buf[bytes]=0;
   if(strcmp(buf,"query_host_name")==0) {
  	  gethostname(buf,1024);
	  sendto(socket,buf,strlen(buf),0,&from,len);
	  DBG(fprintf(stderr, "Reporting host : %s\n",buf); fflush(stderr));
   }
   if(strcmp(buf,"query_host_ip")==0) {
	  char* daddr;
	  struct hostent *ent;
	  gethostname(buf,1024);
  	  ent = gethostbyname(buf);
	  if(ent!=NULL) {
          daddr = inet_ntoa(*(in_addr *)*ent->h_addr_list);
	      if(daddr!=NULL) {
	          sendto(socket,daddr,strlen(daddr),0,&from,len);
	          DBG(fprintf(stderr, "Reporting IP : %s\n",daddr); fflush(stderr));
		  } else DBG(fprintf(stderr, "Can't find IP.\n"); fflush(stderr));
	  } else DBG(fprintf(stderr, "Can't convert name into address.\n"); fflush(stderr));
   }
   if(strcmp(buf,"query_host_all")==0) {
	  char* daddr;
	  struct hostent *ent;
	  gethostname(buf,1024);
  	  ent = gethostbyname(buf);
	  if(ent!=NULL) {
          daddr = inet_ntoa(*(in_addr *)*ent->h_addr_list);
	      if(daddr!=NULL) {
			  char outstr[500];
              sprintf(outstr,"{ {%s} {%s} }",daddr,buf);
	          sendto(socket,outstr,strlen(outstr),0,&from,len);
	          DBG(fprintf(stderr, "Reporting host:IP : %s %s\n",buf,daddr); fflush(stderr));
		  } else DBG(fprintf(stderr, "Can't find IP.\n"); fflush(stderr));
	  } else DBG(fprintf(stderr, "Can't convert name into address.\n"); fflush(stderr));
   }
}

void DoQueryProcessing(int socket) {
timeval timeout;
timeout.tv_sec  = 0;
timeout.tv_usec = 100000;

   
   fd_set socket_set;
   int    nfsd = 0;

#ifndef _WIN32
   nfsd = FD_SETSIZE;
#endif
   FD_ZERO(&socket_set);
   FD_SET(socket, &socket_set);

#ifdef hp700
   if(select(nfsd,(int *)&socket_set,NULL,NULL,&timeout)<=0) return;
#else
   if(select(nfsd,&socket_set,NULL,NULL,&timeout)<=0) return;
#endif
   struct sockaddr s;
   socklen_t s_size = sizeof(s);
   int new_socket = accept(socket, &s, &s_size);
   if(new_socket >= 0) {
        DBG(fprintf(stderr, "Accepting\n"); fflush(stderr));
        ProcessRequests(new_socket, &s);
   }
		
}

void WaitRequest(int socket, int info_socket)
{
    do {
        DoInfoProcessing(info_socket);
		DoQueryProcessing(socket);
    } while(!do_shutdown);
}

void SaveParent(char *addr)
{
    struct hostent *ent = gethostbyname(addr);
    if(ent == NULL) {
	fprintf(stderr, "Unable to find parent host: %s\n", addr);
	fflush(stderr);
	return;
    }
    parent             = new sockaddr_in;
    parent->sin_family = AF_INET;
    parent->sin_port   = htons (ns_port);
    parent->sin_addr   = *(in_addr *)*ent->h_addr_list;
}

void ParseArguments(int argc, char **argv)
{
    int i = 1;
    while(i < argc) {
	if(strcmp(argv[i], "-parent") == 0 && i + 1 < argc) {
	    SaveParent(argv[i + 1]);
	    i++;
	} else if(strcmp(argv[i], "-stop") == 0)
	    do_stop = 1;
	else if(strcmp(argv[i], "-debug") == 0)
	    do_debug = 1;
	i++;
    }
}


// ===========================================================================
#ifdef _WIN32
WINMAIN()
    WORD    wVersionRequested;  
    WSADATA wsaData; 
    int     err; 
   
    wVersionRequested = MAKEWORD(1, 1); 
    err               = WSAStartup(wVersionRequested, &wsaData); 
    if (err != 0) {
	     fprintf(stderr, "no sockets available\n");
	     fflush(stderr);
	     return -1;
    }
#else //not win32
int main(int argc, char** argv) {
#endif // win32

    // Do good daemon stuff:
    chdir("/");
    // More stuff to do: close file handles, fork and become session leader
    // See "Daemon Processes" in Stevens _Advance Programming in Unix Envm't_.

    ParseArguments(argc, argv);

#ifdef _WIN32
	if(do_debug) CreateConsole();
#endif    


    if(do_stop) 	SendStopCommand();
    else {
	    if(AlreadyRunning()) {
	        fprintf(stderr, "Nameserver already running.\n");
		fflush(stderr);
	        return 1;
		}
	    int socket      = CreateListener();
	    int info_socket = CreateDatagramListener();

	    if(socket < 0) {
	        fprintf(stderr, "Unable to create socket.\n");
		fflush(stderr);
	        return -1;
		}
	    WaitRequest(socket,info_socket);
	    closesocket(socket);
	    closesocket(info_socket);
    }
    return 0;
}
