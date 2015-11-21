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
#include <sysent.h>
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
#include "../../nameserver/include/nameServCalls.h"

#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif

static int ns_port = NS_PORT;

//-------------------------------------------------------------------------------------------------

static int SendCommand(int socket, unsigned char cmd)
{
    int res = send(socket, (char *)&cmd, 1, 0);
    if(res <= 0) {
	closesocket(socket);
	return 0;
    }
    return 1;
}


static int SendBuffer(int socket, const char *buf, int len)
{
    char *ptr = (char *)buf;
    while(len != 0){
	int res = send(socket, ptr, len, 0);
	if(res <= 0)
	    return 0;
	ptr += res;
	len -= res;
    }
    return 1;
}

#error network byte order issue: use nameServCalls.h functions instead!
static int SendInteger(int socket, int val)
{
    unsigned char buf[4];
    
    buf[0]  = val & 255;
    buf[1]  = (val >> 8) & 255;
    buf[2]  = (val >> 16) & 255;
    buf[3]  = (val >> 24) & 255;
    return SendBuffer(socket, (char *)buf, 4);
}

static int ReceiveBuffer(int socket, char *buf, int len)
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

#error network byte order issue: use nameServCalls.h functions instead!
static int ReceiveInteger(int socket, int *val)
{
    unsigned char buf[4];

    if(!ReceiveBuffer(socket, (char *)buf, 4))
	return 0;
    *val = buf[0] | ((unsigned)buf[1] << 8) | ((unsigned)buf[2] << 16) | ((unsigned)buf[3] << 24);
    return 1;
}

static int GetNameServerPort(void)
{
    return ns_port;
}

static int ConnectToNameServer(void)
{
    struct sockaddr_in serv_addr;
    char buf[1024];

    if(gethostname(buf, 1024) != 0)
	return -1;
    struct hostent *ent = gethostbyname(buf);
    if(ent == NULL)
	return -1;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons (GetNameServerPort());
    serv_addr.sin_addr   = *(in_addr *)*ent->h_addr_list;
    /* Create the socket. */
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	return sock;
    /* Give the socket a name. */
    if(connect(sock,  (struct sockaddr *)&serv_addr, sizeof (sockaddr_in)) < 0) {
	closesocket(sock);
	return -1;
    }
    return sock;
}

//-------------------------------------------------------------------------------------------------

// Function connects to name server and asks if for specified service
// returns 1 on success and sets up "service_port" and "tcp_addr"
// returns 0 if no service found
// returns -1 if unable to connect to name server
int NameServerGetService(const char *name, int &service_port, int &tcp_addr)
{
    unsigned char len_buf[4];

    int sock = ConnectToNameServer();
    if(sock < 0)
	return -1;
    if(!SendCommand(sock, FIND_SERVER))
	return -1;
    if(!SendInteger(sock, strlen(name))) {
	closesocket(sock);
	return -1;
    }
    if(!SendBuffer(sock, name, strlen(name))) {
	closesocket(sock);
	return -1;
    }
    int res = recv(sock, (char *)len_buf, 1, 0);
    if(res <= 0) {
	closesocket(sock);
	return -1;
    }
    if(len_buf[0] == 0){
	closesocket(sock);
	return 0;
    }
    res = recv(sock, (char *)len_buf, 4, 0);
    if(res <= 0){
	closesocket(sock);
	return -1;
    }
    service_port = len_buf[0] | ((unsigned)len_buf[1] << 8) | ((unsigned)len_buf[2] << 16) | ((unsigned)len_buf[3] << 24);
    res          = recv(sock, (char *)len_buf, 4, 0);
    if(res <= 0){
	closesocket(sock);
	return -1;
    }
    tcp_addr = len_buf[0] | ((unsigned)len_buf[1] << 8) | ((unsigned)len_buf[2] << 16) | ((unsigned)len_buf[3] << 24);
    closesocket(sock);
    return 1;
}

//-------------------------------------------------------------------------------------------------

// Function connects to name server and fills up list of services
// returns 1 on success and "services" gets initialized to list of services
// returns 0 if no service found
// returns -1 if unable to connect to name server
int NameServerGetListOfServices(NameServerServiceList **services)
{
    unsigned char len_buf[4];

    *services = NULL;
    int sock  = ConnectToNameServer();
    if(sock < 0)
	return -1;
    if(!SendCommand(sock,  LIST_OF_SERVERS))
	return -1;
    NameServerServiceList *list = NULL;
    NameServerServiceList *head = NULL;
    int len;
    while(ReceiveInteger(sock, &len) && len != 0) {
	char *buf = new char[len + 1];
	if(!ReceiveBuffer(sock, buf, len)) {
	    closesocket(sock);
	    delete [] buf;
	    return -1;
	}
	buf[len] = 0;
	if(list == NULL) 
	    head = list = new NameServerServiceList(buf);
	else {
	    list->next = new NameServerServiceList(buf);
	    list       = list->next;
	}
    }
    closesocket(sock);
    *services = head;
    return 1;
}

//-------------------------------------------------------------------------------------------------

// Function connects to name server and registers specified service
// name - service name
// port - service port
// returns 1 on success
// returns -1 if unable to connect to name server
int NameServerRegisterService(const char *name, int port)
{
    unsigned char len_buf[4];

    int sock  = ConnectToNameServer();
    if(sock < 0)
	return -1;
    if(!SendCommand(sock, REGISTER_SERVER))
	return -1;
    if(!SendInteger(sock, port)){
	closesocket(sock);
	return -1;
    }
    if(!SendInteger(sock, strlen(name))) {
	closesocket(sock);
	return -1;
    }
    if(!SendBuffer(sock, name, strlen(name))) {
	closesocket(sock);
	return -1;
    }
    closesocket(sock);
    return 1;
}

//-------------------------------------------------------------------------------------------------

