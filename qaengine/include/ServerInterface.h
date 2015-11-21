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
// ServerInterface.h: interface for the ServerInterface class.
//
//////////////////////////////////////////////////////////////////////
#include "globals.h"
#include "../../dish/include/SocketComm.h"
#include "../../nameserver/include/nameServCalls.h"
#include "HitsTable.h"
#ifdef _WIN32
    #include <windows.h>
    #include <winsock.h>
    #include <io.h>
#else
    #ifndef hp10
	#include <sysent.h>
    #endif
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

class ServerInterface  
{
public:
	void disconnect(void);
	HitsSet execQuery(int type,string script, string command, string module, string arguments);
	ScopeSet getBuildScope();
	int connect(string host,string service);
	int isConnected(void);
	ServerInterface();
	virtual ~ServerInterface();

private:
	HitsSet parseNormalHits(string data,string pname);
    HitsSet parseStatisticalHits(string data,string pname);
	int m_BatchScriptSourced;
	string eval(string command);
	int m_Socket;
	vector<string> m_SourcedScripts;
	string m_PreviousModule;
};

