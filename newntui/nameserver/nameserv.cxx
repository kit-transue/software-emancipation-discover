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
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#include <io.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
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

#define NS_PORT 28948

#define REGISTER_SERVER   0
#define LIST_OF_SERVERS   1
#define FIND_SERVER       2
#define UNREGISTER_SERVER 3
#define STOP              4 

int ns_port     = NS_PORT;
int do_shutdown = 0;
int do_stop     = 0;
int do_debug    = 0;
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

int SendCommand(int socket, unsigned char cmd)
{
    int res = send(socket, (char *)&cmd, 1, 0);
    if(res <= 0) {
	closesocket(socket);
	return 0;
    }
    return 1;
}

int SendBuffer(int socket, char *buf, int len)
{
    char *ptr = buf;
    while(len != 0){
	int res = send(socket, ptr, len, 0);
	if(res <= 0)
	    return 0;
	ptr += res;
	len -= res;
    }
    return 1;
}

int SendInteger(int socket, int val)
{
    unsigned char buf[4];
    
    buf[0]  = val & 255;
    buf[1]  = (val >> 8) & 255;
    buf[2]  = (val >> 16) & 255;
    buf[3]  = (val >> 24) & 255;
    return SendBuffer(socket, (char *)buf, 4);
}

int ReceiveBuffer(int socket, char *buf, int len)
{
    while(len != 0){
	int res = recv(socket, buf, len, 0);
	if(res <= 0) 
	    return 0;
	buf += res;
	len -= res;
    }
    return 1;
}

int ReceiveInteger(int socket, int *val)
{
    unsigned char buf[4];

    if(!ReceiveBuffer(socket, (char *)buf, 4))
	return 0;
    *val = buf[0] | ((unsigned)buf[1] << 8) | ((unsigned)buf[2] << 16) | ((unsigned)buf[3] << 24);
    return 1;
}

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
    unsigned int set_option = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&set_option, sizeof(set_option));

    /* Give the socket a name. */
    name.sin_family      = AF_INET;
    name.sin_port        = htons (ns_port);
    name.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	return -1;
    if(listen(sock, 5) < 0)
	return -1;
    return sock;
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
    unsigned char port_buf[4];
    unsigned char len_buf[4];
    int res = recv(socket, (char *)port_buf, 4, 0);
    if(res <= 0)
	return res;
    res = recv(socket, (char *)len_buf, 4, 0);
    if(res <= 0)
	return res;
    int port = port_buf[0] | ((unsigned)port_buf[1] << 8) | ((unsigned)port_buf[2] << 16) | ((unsigned)port_buf[3] << 24);
    int len  = len_buf[0] | ((unsigned)len_buf[1] << 8) | ((unsigned)len_buf[2] << 16) | ((unsigned)len_buf[3] << 24);
    if(len == 0)
	return -1;
    char *name = new char[len + 1];
    char *ptr  = name;
    while(len != 0){
	int res = recv(socket, ptr, len, 0);
	if(res <= 0) {
	    delete [] name;
	    return res;
	}
	ptr += res;
	len -= res;
    }
    *ptr        = 0;
    int success = 1;
    if(parent != NULL)
	success = RegisterServerOnParent(port, name);
    else {
	struct sockaddr_in *in = (sockaddr_in *)addr;
	ServerDescriptor *sd   = ServerDescriptor::LookupName(name);
	if(sd == NULL) {
	    sd = new ServerDescriptor(name, port, in->sin_addr.s_addr);
	    ServerDescriptor::AddNewServer(sd);
#ifndef irix6
#ifndef hp700    
	    DBG(printf("Adding new server: %s %d (%d.%d.%d.%d)\n", name, port, in->sin_addr.S_un.S_un_b.s_b1, in->sin_addr.S_un.S_un_b.s_b2,
		       in->sin_addr.S_un.S_un_b.s_b3, in->sin_addr.S_un.S_un_b.s_b4));
#endif	    
#endif
	} else {
	    sd->Update(port, in->sin_addr.s_addr);
#ifndef irix6
#ifndef hp700
	    DBG(printf("Updating server: %s %d (%d.%d.%d.%d)\n", name, port, in->sin_addr.S_un.S_un_b.s_b1, in->sin_addr.S_un.S_un_b.s_b2,
		       in->sin_addr.S_un.S_un_b.s_b3, in->sin_addr.S_un.S_un_b.s_b4));
#endif
#endif
	}
    }
    delete [] name;
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
    unsigned char len_buf[4];
    int res = recv(socket, (char *)len_buf, 4, 0);
    if(res <= 0)
	return res;
    int len  = len_buf[0] | ((unsigned)len_buf[1] << 8) | ((unsigned)len_buf[2] << 16) | ((unsigned)len_buf[3] << 24);
    if(len == 0)
	return -1;
    char *name = new char[len + 1];
    char *ptr  = name;
    while(len != 0){
	int res = recv(socket, ptr, len, 0);
	if(res <= 0) {
	    delete [] name;
	    return res;
	}
	ptr += res;
	len -= res;
    }
    *ptr = 0;
    if(parent != NULL) {
	UnregisterServerOnParent(name);
    } else {
	ServerDescriptor *sd  = ServerDescriptor::LookupName(name);
	if(sd != NULL) {
	    ServerDescriptor::RemoveServer(name);
	    DBG(printf("Removing server: %s\n", name));
	}
    }
    delete [] name;
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
	if(!SendInteger(socket, 0))
	    return 0;
    } else {
	ServerDescriptor *curr = ServerDescriptor::sd_list;
	while(curr != 0) {   
	    if(!SendInteger(socket, strlen(curr->name)))
		return 0;
	    if(!SendBuffer(socket, curr->name, strlen(curr->name)))
		return 0;
	    curr = curr->next;
	}
	if(!SendInteger(socket, 0))
	    return 0;
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
    name.sin_port        = sd->port;
    name.sin_addr.s_addr = sd->tcp_addr;
    if(connect(sock,  (struct sockaddr *) &name, sizeof (name)) < 0) {
		  int result = WSAGetLastError();
          if(result!=WSAEWOULDBLOCK) return 0;
	}
    DBG(printf("Server alive.\n"));
    closesocket(sock);
    return 1;
}

int SendServerInfo(int socket, ServerDescriptor *sd)
{
    unsigned char len_buf[4];

    len_buf[0] = 1;
    int res    = send(socket, (char *)len_buf, 1, 0);
    if(res <= 0)
	return 0;
    if(!SendInteger(socket, sd->port))
	return 0;
    if(!SendInteger(socket, sd->tcp_addr))
	return 0;
    return 1;
}

ServerDescriptor *FindServerOnParent(char *name)
{
    unsigned char len_buf[4];

    int psock = ConnectToParent();
    if(psock <= 0)
	return NULL;
    if(!SendCommand(psock, FIND_SERVER))
	return NULL;
    if(!SendInteger(psock, strlen(name))) {
	closesocket(psock);
	return NULL;
    }
    if(!SendBuffer(psock, name, strlen(name))) {
	closesocket(psock);
	return NULL;
    }
    int res = recv(psock, (char *)len_buf, 1, 0);
    if(res <= 0 || len_buf[0] == 0){
	closesocket(psock);
	return NULL;
    }
    res = recv(psock, (char *)len_buf, 4, 0);
    if(res <= 0){
	closesocket(psock);
	return NULL;
    }
    int port = len_buf[0] | ((unsigned)len_buf[1] << 8) | ((unsigned)len_buf[2] << 16) | ((unsigned)len_buf[3] << 24);
    res = recv(psock, (char *)len_buf, 4, 0);
    if(res <= 0){
	closesocket(psock);
	return NULL;
    }
    int tcp_addr         = len_buf[0] | ((unsigned)len_buf[1] << 8) | ((unsigned)len_buf[2] << 16) | ((unsigned)len_buf[3] << 24);
    ServerDescriptor *sd = new ServerDescriptor("", port, tcp_addr);
    closesocket(psock);
    return sd;
}

int DoFindServer(int socket)
{
    unsigned char len_buf[4];

    int res = recv(socket, (char *)len_buf, 4, 0);
    if(res <= 0)
	return res;
    int len  = len_buf[0] | ((unsigned)len_buf[1] << 8) | ((unsigned)len_buf[2] << 16) | ((unsigned)len_buf[3] << 24);
    if(len == 0)
	return -1;
    char *name = new char[len + 1];
    char *ptr  = name;
    while(len != 0){
	int res = recv(socket, ptr, len, 0);
	if(res <= 0) {
	    delete [] name;
	    return res;
	}
	ptr += res;
	len -= res;
    }
    *ptr = 0;
    if(parent) {
	ServerDescriptor *sd = FindServerOnParent(name);
	delete [] name;
	if(sd) {
	    int success = SendServerInfo(socket, sd);
	    delete sd;
	    return success;
	} else {
	    len_buf[0] = 0;
	    res        = send(socket, (char *)len_buf, 1, 0);
	    return res;
	}
    } else {
	DBG(printf("Looking for name : %s\n", name));
	ServerDescriptor *sd = ServerDescriptor::LookupName(name);
	if(sd) {
	    int success = TryServer(sd);
	    if(!success) {
		ServerDescriptor::RemoveServer(name);
		DBG(printf("Removing dead server: %s\n", name));
		sd = NULL;
	    }
	}
	delete [] name;
	if(sd == NULL){
	    len_buf[0] = 0;
	    res        = send(socket, (char *)len_buf, 1, 0);
	    return res;
	}
	int success = SendServerInfo(socket, sd);
	return success;
    }
}

void SendStopCommand(void)
{
    struct sockaddr_in self_addr;

    struct hostent *ent = gethostbyname("127.0.0.1");
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
    if(connect(sock,  (struct sockaddr *)&self_addr, sizeof (sockaddr_in)) < 0)
	return;
    SendCommand(sock, STOP);
    closesocket(sock);
}

void ProcessRequests(int socket, sockaddr *addr)
{
    char req_buf[1];

    int res = recv(socket, req_buf, 1, 0);
    if(res <= 0)
	return;
    if(req_buf[0] == REGISTER_SERVER)
	DoRegisterServer(socket, addr);
    else if(req_buf[0] == LIST_OF_SERVERS) 
	DoListOfServers(socket);
    else if(req_buf[0] == FIND_SERVER)
	DoFindServer(socket);
    else if(req_buf[0] == UNREGISTER_SERVER)
	DoUnregisterServer(socket);
    else if(req_buf[0] == STOP)
	do_shutdown = 1;
    closesocket(socket);
}

void WaitRequest(int socket)
{
    do {
	struct sockaddr s;
	int s_size = sizeof(s);
	int new_socket = accept(socket, &s, &s_size);
	if(new_socket >= 0) {
	    DBG(printf("Accepting\n"));
	    ProcessRequests(new_socket, &s);
	}
    } while(!do_shutdown);
}

void SaveParent(char *addr)
{
    struct hostent *ent = gethostbyname(addr);
    if(ent == NULL) {
	printf("Unable to find parent host: %s\n", addr);
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

int main(int argc, char **argv) 
{
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
#endif    
    ParseArguments(argc, argv);
    if(do_stop)
	SendStopCommand();
    else {
	int socket = CreateListener();
	if(socket < 0) {
	    printf("Unable to create socket.\n");
	    return -1;
	}
	WaitRequest(socket);
	closesocket(socket);
    }
    return 0;
}
