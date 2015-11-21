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
#include <fstream.h>
#ifdef _WIN32
#include <afxsock.h>
#include <afxtempl.h>
#include <windows.h>
#include <winsock.h>
#include <io.h>
#else
#include <strings.h>
#include <sysent.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
#include "../../sockets/include/SocketComm.h"

#ifndef _WIN32
#define closesocket(sock) close(sock)

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

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

//---------------------------------------------------------------------------------------------------

static unsigned char GeneratePacketNumber(void)
{
    static unsigned char pkt = 0;
    pkt++;
    return pkt;
}

//---------------------------------------------------------------------------------------------------

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
    return SendBuffer(socket, (const char *)buf, 4);
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
#ifdef _WIN32
    int tmp_switch = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
#endif
    return sock;    
}

#ifdef _WIN32

IMPLEMENT_DYNAMIC(CConnection, CSocket)

// CSocket stuff begins here 
CConnection::CConnection()
{
	m_bConnectionClosed = TRUE;
}

void CConnection::OnReceive(int nErrorCode)
{
	m_bConnectionClosed = FALSE;
	CSocket::OnReceive(nErrorCode);
}

void CConnection::OnClose(int nErrorCode)
{
	m_bConnectionClosed = TRUE;
	CSocket::OnClose(nErrorCode);
}

void CConnection::Close(void)
{
	m_bConnectionClosed = TRUE;
	CSocket::Close();
}


BOOL CConnection::IsConnectionClosed(void) 
{
	return m_bConnectionClosed;
}

void CConnection::SetConnectionClosed(BOOL closed) 
{
	m_bConnectionClosed = closed;
}

bool CConnection::ConnectToServerEx(int port, int tcp_addr) 
{
    struct sockaddr_in serv_addr;

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_port        = htons(port);
    serv_addr.sin_addr.s_addr = htonl(tcp_addr);
    /* Create the socket. */
	if ( !Create(0, SOCK_STREAM, 0) ) {
		return false;
	}
    /* Give the socket a name. */
    if( Connect((struct sockaddr *)&serv_addr, sizeof (sockaddr_in)) == FALSE) {
		Close();
		return false;
    }
    int tmp_switch = 1;
	setsockopt(m_hSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
	setsockopt(m_hSocket, IPPROTO_TCP, SO_REUSEADDR, (char *)&tmp_switch, sizeof(tmp_switch));
	SetConnectionClosed(FALSE);
    return true;    
}

int CConnection::SendCommand(DISCommand *cmd)
{
    unsigned char pkt_num = GeneratePacketNumber();
    int success           = Send((const char *)&pkt_num, 1, 0);
    if(success <= 0)
	return 0;
    success = Send(&cmd->type, 1, 0);
    if(success <= 0)
	return 0;
    success = SendInteger(strlen(cmd->txt) + 1);
    if(success <= 0)
	return 0;
    success = SendBuffer(cmd->txt, strlen(cmd->txt));
    if(success <= 0)
	return 0;
    char tmp = 0;
    success = Send(&tmp, 1, 0);
    if(success <= 0)
	return 0;
    return 1;
}

int  CConnection::ReceiveReply(char **buf, unsigned char &result_code)
{
    char          packet_type;
    char          *result    = NULL;
    int           len        = 0;
    int           actual_len = 0;
    do {
	char *tmp;
	int success = ReceivePacket(packet_type, result_code, &tmp);
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

    return  1;
}

//---------------------------------------------------------------------------------------------------

int CConnection::ReceivePacket(char &packet_type, unsigned char &result, char **buf)
{
    unsigned char packet_num;
    int success = Receive((char *)&packet_num, 1, 0);
    if(success <= 0) 
	return 0;
    success = Receive(&packet_type, 1, 0);
    if(success <= 0) 
	return 0;
    success = Receive((char *)&result, 1, 0);
    if(success <= 0) 
	return 0;
    int len;
    success = ReceiveInteger(&len);
    if(success <= 0) 
	return 0;
    char *tmp = new char[len];
    success   = ReceiveBuffer(tmp, len);
    if(success <= 0) {
	delete [] tmp;
	return 0;
    }
    *buf = tmp;
    return 1;
}

//---------------------------------------------------------------------------------------------------

int CConnection::SendBuffer(const char *buf, int len)
{
    char *ptr = (char *)buf;
    while(len != 0){
	int res = Send(ptr, len, 0);
	if(res <= 0)
	    return 0;
	ptr += res;
	len -= res;
    }
    return 1;
}

int CConnection::SendInteger(int val)
{
    unsigned char buf[4];
    
    buf[0]  = val & 255;
    buf[1]  = (val >> 8) & 255;
    buf[2]  = (val >> 16) & 255;
    buf[3]  = (val >> 24) & 255;
    return SendBuffer((const char *)buf, 4);
}

int CConnection::ReceiveBuffer(char *buf, int len)
{
    while(len != 0){
	int res = Receive(buf, len, 0);
	if(res <= 0) 
	    return 0;
	buf += res;
	len -= res;
    }
    return 1;
}

int CConnection::ReceiveInteger(int *val)
{
    unsigned char buf[4];

    if(!ReceiveBuffer((char *)buf, 4))
	return 0;
    *val = buf[0] | ((unsigned)buf[1] << 8) | ((unsigned)buf[2] << 16) | ((unsigned)buf[3] << 24);
    return 1;
}

#endif

