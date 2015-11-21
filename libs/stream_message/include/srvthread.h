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
#ifndef TCP_SERVERTHREAD_H
#define TCP_SERVERTHREAD_H

#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#include <io.h>
#else
#include <strings.h>
#ifndef hp10
#include <sysent.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#ifdef irix6
#include <sys/poll.h>
#endif
#endif
 
#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif

#include "threads.h"
#include "aconnectn.h"

// private header, solely for implementation of transport module
namespace nTransport {

template<typename T>
class Communicator;

// ===========================================================================
// Implement a server thread, activated when 'accept' creates a connection.
// 'server' follows the service protocol.  Calls back the 'run' member
// function when the thread is started, and we provide a Connection object
// to the server.
//
template<typename T>
class ServerThread : public SAThreads::Procedure {
public:
    ServerThread(int connected_socket, Communicator<T> &server, SAThreads::Thread *t)
      : socket(connected_socket),
	server(server),
	theThread(t)
    {}
    virtual void run()
    {
	delete theThread; // Can't track the Thread, and it's useless.
	{
	    nTransport::ActualConnection<T> conn(server, socket);
	    server.communicate(conn);
	}
	delete this;
    }
private:
    int socket;
    Communicator<T> &server;
    SAThreads::Thread *theThread;
};
}
#endif // TCP_SERVERTHREAD_H
