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
#include "transport.h"
#include "aconnectn.h"

#ifdef hp10
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef ISO_CPP_HEADERS
#include <fstream>
#else
#include <fstream.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#include <io.h>
#else
#include <strings.h>
#include <sys/socket.h>
#endif

#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif

using namespace nTransport;

// ===========================================================================
// code taken from dish/src/SocketComm.C
//
// returns <=0 in case of failure; 1 in case of success
//
int nTransport::send(int socket, char *ptr, int len)
{
    while(len != 0){
	int res = ::send(socket, ptr, len, 0);
	if(res <= 0)
	    return res;
	ptr += res;
	len -= res;
    }
    return 1;
}

// ===========================================================================
//
int nTransport::receiveBuffer(int socket, char *buf, int len)
{
#if defined(hp10) && defined(TRANSPORT_SERVER)
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(socket, &rset);
#endif
    while(len != 0) {
#if defined(hp10) && defined(TRANSPORT_SERVER)
	// Avoid seeming HPUX bug in which 'recv' would block
	// and allow another thread to starve (mbdriver).
	select(socket+1, &rset, NULL, NULL, NULL);
#endif
	int res = ::recv(socket, buf, len, 0);
	if(res <= 0) 
	    return res;
	buf += res;
	len -= res;
    }
    return 1;
}

// ===========================================================================
int nTransport::closesock(int socket) {
    return ::closesocket(socket);
}
