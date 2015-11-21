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
namespace std {}
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
#include <io.h>
#include <stdint.h>
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

#include "../include/nameServCalls.h"
#include "debug.h"

#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif

static int ns_port = NS_PORT;

//-------------------------------------------------------------------------------------------------

// XXX: these have strange return codes: 1 is success.  Should invert, or
// possibly convert to exceptions.
int SendCommand(int socket, unsigned char cmd)
{
    int res = send(socket, (char *)&cmd, 1, 0);
    if(res <= 0) {
	closesocket(socket);
	return 0;
    }
    return 1;
}

int ReceiveCommand(int socket, unsigned char *cmd)
{
    int res = recv(socket, (char *)cmd, 1, 0);
    if(res <= 0) {
	closesocket(socket);
	return 0;
    }
    return 1;
}

int SendBuffer(int socket, char const *buf, int len)
{
    char const *ptr = buf;
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
    uint32_t buf = htonl(val);
    return SendBuffer(socket, (char *)&buf, sizeof(buf));
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
    uint32_t buf;

    if(!ReceiveBuffer(socket, (char *)&buf, sizeof(buf)))
	return 0;
    *val = ntohl(buf);
    return 1;
}

//------------------------------------------------------------------------------------------------------
// This function will check writability of the selected socket.
// Actually it means: does client behind socket exist?
//------------------------------------------------------------------------------------------------------
int CheckSocketForWrite(int socket) {
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
    int res = select(nfsd,NULL,(int *)&sock_set,NULL,&timeout);
#else
    int res = select(nfsd,NULL,&sock_set,NULL,&timeout);
#endif
    if(res > 0) {
        return 1;
    }
    return 0;
}

int SendString(int socket, char const *str)
{
    size_t len = (str != NULL) ? strlen(str) : 0;

    // Check this socket before sending data thru it.
    if(CheckSocketForWrite(socket)) {

        // Send size of this string
        if(!SendInteger(socket, len)) {
            return 0;
        }

        // Send string itself
        if(!SendBuffer(socket,str,len)) {
            return 0;
        }

        // Print confirmation to log-file.
        _DBG(fprintf(log_fd, "\n<SEND>\n"));
        _DBG(fprintf(log_fd, "%s", str));
        _DBG(fprintf(log_fd, "\n</SEND>\n\n"));

    } else {
        _DBG(fprintf(log_fd,
                     "Couldn't send data to socket %d! This socket is broken.\n",
                     socket));
        return 0;
    }
    return 1;
}

//---------------------------------------------------------------------------------------------
// This function gets string from socket. It receives the length of message at first and 
// then allocates necessary amount of memory for string and copies received command to
// this memory.
//---------------------------------------------------------------------------------------------
char* ReceiveString(int socket) {
    static char* data=NULL;;
 
    int len;

    int rc = ReceiveInteger(socket, &len);
    if(rc<=0) 
        return NULL;
    if(data!=NULL) 
        delete data;
    data = new char [len+1];
    rc = ReceiveBuffer(socket, data, len);
    if(rc<=0) 
        return NULL;
    data[len]=0;

    _DBG(fprintf(log_fd, "\n<RECEIVE>\n"));
    _DBG(fprintf(log_fd, "%s", data));
    _DBG(fprintf(log_fd, "\n</RECEIVE>\n\n"));

    return data;
}


int GetNameServerPort(void)
{
    return ns_port;
}

#ifdef _WIN32
// Sleep for approximately, but not exactly, the given number of
// milliseconds.  We're trying to avoid collisions.
void sloppy_sleep(int millisecs, int n) {
    static unsigned int p = (unsigned int)GetCurrentProcessId();
    // A scrambled mix of pid and n, between 0 and 255:
    unsigned short mix
        = (((p * 11035152 + 12345 + n) * 110351252) >> 16) & 0xff;
    Sleep(millisecs + mix - 100);
}
#endif

int ConnectToNameServer(void)
{
    struct sockaddr_in serv_addr;

#ifdef XXX_dont_use_loopback_address
    char buf[1024];

    if(gethostname(buf, 1024) != 0)
	return -1;
    struct hostent *ent = gethostbyname(buf);
#else
    u_long addr = inet_addr("127.0.0.1");
    struct hostent *ent = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
#endif

    if(ent == NULL)
	return -1;
    int sock;
    for (int n = 1; n <= 4; n += 1) {
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port   = htons (GetNameServerPort());
	serv_addr.sin_addr   = *(in_addr *)*ent->h_addr_list;
	/* Create the socket. */
	sock = socket (PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	    return sock;
	/* Give the socket a name. */
	int connres;
	if((connres = connect(sock,  (struct sockaddr *)&serv_addr, sizeof (sockaddr_in))) < 0) {
#ifdef _WIN32
	    if (WSAGetLastError() == WSAECONNREFUSED && n <= 3) {
		// Have patience with Windows workstation, and try again.
	        closesocket(sock);
		sloppy_sleep(n*300, n);
		continue;
	    }
#endif
	    closesocket(sock);
	    return -1;
	}
	else {
	    break;
	}
    }
    return sock;
}


static int IsIp(const char* host) {
int i;
int dcount;
int result;

    result = 1;
    dcount = 0;
    for(i=0;i<(int)strlen(host);i++) {
		if(host[i]=='.') {
			dcount++;
			continue;
		}
                if(host[i] < '0' || host[i] > '9') {
			result = 0;
			break;
		}
	}
	if(dcount!=3) result = 0;
	return result;
}

static unsigned MakeIP(const char* host) {
int i,j,k;
char val[10];
union {
	struct {
       unsigned char b1;
       unsigned char b2;
       unsigned char b3;
       unsigned char b4;
	} s_unb;
	unsigned long s_ad;
}conv;

    k=0;
	j=0;
    for(i=0;i<(int)strlen(host);i++) {
		if(host[i]=='.') {
			val[j]=0;
			switch(k) {
			   case 0 : conv.s_unb.b1 = atoi(val); break;
			   case 1 : conv.s_unb.b2 = atoi(val); break;
			   case 2 : conv.s_unb.b3 = atoi(val); break;
			   case 3 : conv.s_unb.b4 = atoi(val); break;
			}
			j=0;
			val[j]=0;
			k++;
			continue;
		}
		val[j++]=host[i];
	}
	val[j]=0;
	if(j>0) {
			switch(k) {
			   case 0 : conv.s_unb.b1 = atoi(val); break;
			   case 1 : conv.s_unb.b2 = atoi(val); break;
			   case 2 : conv.s_unb.b3 = atoi(val); break;
			   case 3 : conv.s_unb.b4 = atoi(val); break;
			}
	}
	return conv.s_ad;
}


int ConnectToNameServerRemote(const char* host)
{
	struct sockaddr_in serv_addr;
//    char buf[1024];

    if(IsIp(host)==0) {
        struct hostent *ent;
        ent = gethostbyname(host);
        if(ent == NULL) return -1;
            serv_addr.sin_addr   = *(in_addr *)*ent->h_addr_list;
	} else {
            serv_addr.sin_addr.s_addr = MakeIP(host);
	}
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons (GetNameServerPort());
	
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
    unsigned char c;

    int sock = ConnectToNameServer();
    if(sock < 0)
	return -1;
    if(!SendCommand(sock, FIND_SERVER))
	return -1;
    if(!SendString(sock, name)) {
	closesocket(sock);
	return -1;
    }

    if(!ReceiveCommand(sock, &c)) {
        closesocket(sock);
	return -1;
    }
    if(c == 0){
	closesocket(sock);
	return 0;
    }

    if(!ReceiveInteger(sock, &service_port)) {
	closesocket(sock);
	return -1;
    }

    if(!ReceiveInteger(sock, &tcp_addr)) {
	closesocket(sock);
	return -1;
    }
    closesocket(sock);
    return 1;
}


// Function connects to name server and asks if for specified service
// returns 1 on success and sets up "service_port" and "tcp_addr"
// returns 0 if no service found
// returns -1 if unable to connect to name server
int NameServerGetServiceRemote(const char *host, const char *name, int &service_port, int &tcp_addr)
{
    unsigned char cmd;

    int sock = ConnectToNameServerRemote(host);
    if(sock < 0)
	return -1;
    if(!SendCommand(sock, FIND_SERVER))
	return -1;
    if(!SendString(sock, name)) {
	closesocket(sock);
	return -1;
    }

    if(!ReceiveCommand(sock, &cmd)) {
	closesocket(sock);
	return -1;
    }
    if(cmd == 0){
	closesocket(sock);
	return 0;
    }
    if(!ReceiveInteger(sock, &service_port)) {
	closesocket(sock);
	return -1;
    }
    if(!ReceiveInteger(sock, &tcp_addr)) {
	closesocket(sock);
	return -1;
    }
    closesocket(sock);
    return 1;
}

//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
// Function connects to name server and fills up list of services
// returns 1 on success and "services" gets initialized to list of services
// returns 0 if no service found
// returns -1 if unable to connect to name server
int NameServerGetListOfServices(NameServerServiceList **services)
{
    *services = NULL;
    int sock  = ConnectToNameServer();
    if(sock < 0)
	return -1;
    if(!SendCommand(sock, LIST_OF_SERVERS))
	return -1;
    NameServerServiceList *list = NULL;
    NameServerServiceList *head = NULL;
    char *buf;
    while((buf = ReceiveString(sock)) && buf[0] != 0) {
	if(list == NULL) 
	    head = list = new NameServerServiceList(buf);
	else {
	    list->next = new NameServerServiceList(buf);
	    list       = list->next;
	}
    }
    if (buf == NULL) {
        return -1;
    }
    closesocket(sock);
    *services = head;
    return head != NULL;
}

// Function connects to name server and fills up list of services
// returns 1 on success and "services" gets initialized to list of services
// returns 0 if no service found
// returns -1 if unable to connect to name server
int NameServerGetListOfServicesRemote(const char *host, NameServerServiceList **services)
{
    *services = NULL;
    int sock  = ConnectToNameServerRemote(host);
    if(sock < 0) return -1;
    if(!SendCommand(sock,  LIST_OF_SERVERS)) return -1;
    NameServerServiceList *list = NULL;
    NameServerServiceList *head = NULL;
    char *buf;
    while((buf = ReceiveString(sock)) && buf[0] != 0) {
        if(list == NULL) 
            head = list = new NameServerServiceList(buf);
        else {
            list->next = new NameServerServiceList(buf);
            list       = list->next;
        }
    }
    if (buf == NULL) {
        return -1;
    }
    closesocket(sock);
    *services = head;
    return services != NULL;
}



//-------------------------------------------------------------------------------------------------

// Function connects to name server and registers specified service
// name - service name
// port - service port
// returns 1 on success
// returns -1 if unable to connect to name server
int NameServerRegisterService(const char *name, int port)
{
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


  
// Function connects to name server and unregisters specified service
// name - service name
// returns 1 on success
// returns -1 if unable to connect to name server
int NameServerUnRegisterService(const char *name)
{
    int sock  = ConnectToNameServer();
    if(sock < 0)
	return -1;
    if(!SendCommand(sock, UNREGISTER_SERVER))
	return -1;
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

