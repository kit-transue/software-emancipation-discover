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
#include <stdlib.h>
#include <ctype.h>
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#include <cstdlib>
#include <cctype>
#include <fstream>
#endif /* ISO_CPP_HEADERS */
#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#include <io.h>
#else
#include <strings.h>
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

#include "SocketComm.h"
#include "nameServCalls.h"


#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif


//---------------------------------------------------------------------------------------------------

DISCommand::DISCommand(char *t, char tp)
{
    txt  = strdup(t);
    type = tp;
}

DISCommand::~DISCommand()
{
    if(txt)
	free(txt);
}

static unsigned char GeneratePacketNumber(void)
{
    static unsigned char pkt = 0;
    pkt++;
    return pkt;
}

int SendCommand(int socket, DISCommand *cmd)
{
    unsigned char pkt_num = GeneratePacketNumber();
    int success           = send(socket, (const char *)&pkt_num, 1, 0);
    if(success <= 0)
	return 0;
    success = send(socket, &cmd->type, 1, 0);
    if(success <= 0)
	return 0;
    success = SendInteger(socket, strlen(cmd->txt) + 1);
    if(success <= 0)
	return 0;
    success = SendBuffer(socket, cmd->txt, strlen(cmd->txt));
    if(success <= 0)
	return 0;
    char tmp = 0;
    success = send(socket, &tmp, 1, 0);
    if(success <= 0)
	return 0;
    return 1;
}

//----------------------------------------------------------------------------------------------

int ReceivePacket(int socket, char &packet_type, unsigned char &result, char **buf)
{
    unsigned char packet_num;
    int success = recv(socket, (char *)&packet_num, 1, 0);
    if(success <= 0) 
	return 0;
    success = recv(socket, &packet_type, 1, 0);
    if(success <= 0) 
	return 0;
    success = recv(socket, (char *)&result, 1, 0);
    if(success <= 0) 
	return 0;
    int len;
    success = ReceiveInteger(socket, &len);
    if(success <= 0) 
	return 0;
    char *tmp = new char[len];
    success   = ReceiveBuffer(socket, tmp, len);
    if(success <= 0) {
	delete [] tmp;
	return 0;
    }
    *buf = tmp;
    return 1;
}

int ReceiveReply(int socket, char **buf, unsigned char &result_code)
{
    char          packet_type;
    char          *result    = NULL;
    int           len        = 0;
    int           actual_len = 0;
    do {
	char *tmp;
	int success = ReceivePacket(socket, packet_type, result_code, &tmp);
	if(success != 1) {
	    if(result)
		delete [] result;
	    return 0;
	}
	if(result == NULL) {
	    result     = tmp;
	    len        = strlen(tmp) + 1;
	    actual_len = len;
	} else {
	    int tmp_len = strlen(tmp);
	    if(tmp_len + len + 1 > actual_len){
		while(actual_len < tmp_len + len + 1)
		    actual_len += actual_len;
		char *new_result = new char[actual_len];
		if(new_result == NULL)
		    return 0;
		strcpy(new_result, result);
		delete [] result;
		result = new_result;
	    }
	    strcpy(result + len - 1, tmp);
	    len += tmp_len;
	    delete [] tmp;
	}
    } while(packet_type != DATA_FINAL_PACKET);
    *buf = result;
    return 1;
}

//----------------------------------------------------------------------------------------------

int ConnectToServer(int port, int tcp_addr)
{
    struct sockaddr_in serv_addr;

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_port        = htons(port);
    serv_addr.sin_addr.s_addr = htonl(tcp_addr);
    /* Create the socket. */
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	return sock;
    /* Give the socket a name. */
    if(connect(sock,  (struct sockaddr *)&serv_addr, sizeof (sockaddr_in)) < 0) {
	closesocket(sock);
	return -1;
    }
    int tmp_switch = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
    return sock;    
}

int CreateBroadcast(int port) {
struct sockaddr_in   /*srv_addr,*/cli_addr;
int                  cli_sock;

    /* Create client-side datagram socket */
    cli_sock=socket(PF_INET,SOCK_DGRAM,0);
    if (cli_sock<0) {  
	    return -1;
	}

    cli_addr.sin_family=AF_INET;
    cli_addr.sin_addr.s_addr=INADDR_ANY;        
    cli_addr.sin_port=0;                /* no specific local port req'd */

    /* Bind local socket */
    if (bind(cli_sock,(struct sockaddr *)&cli_addr,sizeof(cli_addr))<0){
		return -1;
	}
	int val = 1;
    setsockopt(cli_sock,SOL_SOCKET,SO_BROADCAST,(const char *)&val,sizeof(val));

    return cli_sock;    
}
