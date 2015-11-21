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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef ISO_CPP_HEADERS
#include <fstream>
#include <sstream>
#include <iostream>
#else
#include <fstream.h>
#include <strstream.h>
#include <iostream.h>
#endif
#include <string>
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
#ifdef irix6
#include <sys/poll.h>
#endif
#endif
#ifndef _WIN32
#include <sys/errno.h>
#endif
#include <errno.h>

#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif

#ifdef TRANSPORT_SERVER
#include "threads.h"
using namespace SAThreads;
#endif

using namespace nTransport;
namespace std {}
using namespace std;
#include "aconnectn.h"
#include "service.h"

#ifdef TRANSPORT_SERVER
static Mutex creation_mutex;
#endif

// On Windows, closesocket is sufficient to interrupt the listening thread.
// On Solaris this was insufficient, and it was necessary to make an
// extra connection to get 'accept' to return.
#if defined(_WIN32)
#define NUDGE_TO_SHUT 0
#else
#define NUDGE_TO_SHUT 1
#endif

// ===========================================================================
// -- code from paraset/src/dish/src/SocketComm.C --
//
static int ConnectToServer(int port, int tcp_addr, string **diag)
{
    struct sockaddr_in serv_addr;

    if (diag != NULL) {
	*diag = NULL;
    }
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_port        = htons(port);
    serv_addr.sin_addr.s_addr = htonl(tcp_addr);
    /* Create the socket. */
    int sock;
    {
#ifdef TRANSPORT_SERVER
	Exclusion s(creation_mutex);
#endif
        sock = socket (PF_INET, SOCK_STREAM, 0);
    }
    if (sock < 0) {
	if (diag != NULL) {
	    *diag = new string("failed to create socket");
	}
	return sock;
    }
    /* Give the socket a name. */
    if (connect(sock, (struct sockaddr *)&serv_addr,
                      sizeof (sockaddr_in)           ) < 0) {
	closesocket(sock);
	if (diag != NULL) {
#ifdef ISO_CPP_HEADERS
	    stringstream d;
#else
	    strstream d;
#endif
	    d << "failed on call to connect, with port "
	      << port << " and tcp_addr " << tcp_addr;
#ifndef ISO_CPP_HEADERS
	    d << ends;
#endif
	    *diag = new string(d.str());
	}
	return -1;
    }
    return sock;    
}

#ifdef TRANSPORT_SERVER
// ===========================================================================
// This procedure will create a socket and set it to close on exec.
// -- code from paraset/src/serverspy/serverspy.cpp --
//
static int create_socket() {
    Exclusion s(creation_mutex);
    int sock = socket (PF_INET, SOCK_STREAM, 0);

#ifndef _WIN32
    if (sock >= 0) {
	fcntl(sock, F_SETFD, 1);
    }
#endif

    return sock;
}

// ===========================================================================
// This procedure will create a listen socket for client requests 
// channel and for admin requests channel.
// -- code from paraset/src/serverspy/serverspy.cpp --
//
static int CreateListener() {
    int sock;
    struct sockaddr_in name;
    /* Create the socket. */
    sock = create_socket();
    if (sock < 0) return -1;

    // unsigned int set_option = 1;
    // setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&set_option, sizeof(set_option));

    /* Give the socket a name. */
    name.sin_family      = AF_INET;
    name.sin_port        = 0;
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	return -1;
    if(listen(sock, 5) < 0) return -1;
    return sock;

}

// ===========================================================================
class ActualListener : public Listener {
public:
    ActualListener(int sock) : listeningSocket(sock) {}
    virtual void shut();
    int socket();
private:
    Mutex mutex;
    int listeningSocket;
};

// ===========================================================================
void ActualListener::shut()
{
    int listening_socket;
    {
	Exclusion for_listeningSocket(mutex);
        listening_socket = listeningSocket;
	listeningSocket = -1;
    }
    if (listening_socket >= 0) {
#if NUDGE_TO_SHUT
	struct sockaddr_in sock_info;
	socklen_t len = sizeof(sock_info);
	int res = getsockname(listening_socket, (struct sockaddr *)&sock_info, &len);

	// Windows shuts down the listener when you closesocket on it,
	// but on Sun it needs a nudge.  Make a connection.
	if (res == 0) {
	    int port = ntohs(sock_info.sin_port);
	    int conn_sock = ConnectToServer(port,
				            0x7f000001, // 127.0.0.1 is loopback.
				            NULL);
	    if (conn_sock >= 0) {
		// Connected OK.
		closesocket(conn_sock);
	    }
	}
#else  // !NUDGE_TO_SHUT
        closesocket(listening_socket);
#endif
    }
}

// ===========================================================================
int ActualListener::socket()
{
    int result;
    {
	Exclusion sock_lock(mutex);
	result = listeningSocket;
    }
    return result;
}

// ===========================================================================
// -- code from paraset/src/serverspy/serverspy.cpp --
//
bool nTransport::listen(const ServiceName &service, void *serverp,
	    void (*startServer)(int, void *),
	    void (*ready)(void *, Listener &),
	    string **diagnostic)
{
    if (diagnostic != NULL) {
	*diagnostic = NULL;
    }
    const char *nm = service.getName();
    if (!initialize()) {
	if (diagnostic != NULL) {
	    *diagnostic = new string("Failed to initialize sockets.");
	}
	return false;
    }
    if (nm == NULL) {
	fprintf(stderr, "Cannot start nameless service.\n");
	fflush(stderr);
	if (diagnostic != NULL) {
	    *diagnostic = new string("Failed to initialize sockets.");
	}
	return false;
    }

    // Make listening socket.
    ActualListener listener(CreateListener());
    int original_listening_socket = listener.socket();
    if (original_listening_socket < 0) {
	fprintf(stderr, "Unable to create socket for %s service.\n", nm);
	fflush(stderr);
	if (diagnostic != NULL) {
	    *diagnostic = new string("Unable to create socket.");
	}
	return false;
    }

    if (!service.doRegister(listener.socket())) {
	closesocket(listener.socket());
	if (diagnostic != NULL) {
	    *diagnostic = new string("Unable to register service with nameserver.");
	}
	return false;
    }
    ready(serverp, listener);

    struct sockaddr s;
    int listening_socket;
    int acounter = 0;
    while ((listening_socket = listener.socket()) >= 0) {
	acounter += 1;
	socklen_t s_size = sizeof(s);
        int connected_socket = accept(listening_socket, &s, &s_size); /* blocks */
	int eno = errno;
	if (connected_socket >= 0) {
	    startServer(connected_socket, serverp);
	}
    }

    // Shut down.
    service.unregister();
    if (NUDGE_TO_SHUT || (listening_socket = listener.socket()) >= 0) {
	closesocket(original_listening_socket);
    }
    return true;
}
#endif

// ===========================================================================
// Connect to a given service, calling back client.communicate()
// once the connection is made.  When client.communicate() returns,
// end the connection.
//
// Returns false if no connection was made.
//
bool nTransport::connect_generic(
			  const ServicePort &service, void *clientp,
			  void (*clientCommunicate)(void *, int, void *),
			  string **diagnostic,
			  void *connectionp)
{
    if (diagnostic != NULL) {
	*diagnostic = NULL;
    }
    bool isOK = false;
    int port;
    int addr;
    if (!initialize()) {
	if (diagnostic != NULL) {
	    *diagnostic = new string("Failed to initialize sockets.");
	}
    }
    else if (!service.getPortAndAddr(port, addr, diagnostic)) {
	if (diagnostic != NULL) {
	    if (*diagnostic == NULL) {
		*diagnostic = new string("Failed to get port and address for service.");
	    }
	    else {
		string svc_diag = **diagnostic;
		**diagnostic = "Failed to get port and address for service, because ";
		(*diagnostic)->append(svc_diag);
		(*diagnostic)->append(".");
	    }
	}
    }
    else {
	string *diag;
	int connected_sock = ConnectToServer(port, addr, &diag);
	if (connected_sock >= 0) {
    	    isOK = true;

	    // Process the connection.
	    clientCommunicate(clientp, connected_sock, connectionp);
	}
	else {
	    if (diagnostic != NULL) {
		*diagnostic = new string("Failed to make connection to service");
		if (diag != NULL) {
		    (*diagnostic)->append(", because ");
		    (*diagnostic)->append(*diag);
		    delete diag;
		}
		(*diagnostic)->append(".");
	    }
	}
    }
    return isOK;
}

static bool transport_initted = false;

// ===========================================================================
// intialization of sockets
// returns whether initialized OK
//
bool nTransport::initialize()
{
#ifdef TRANSPORT_SERVER
    Exclusion s(creation_mutex);
#endif
    if (!transport_initted) {
	transport_initted = true;
#ifdef _WIN32
	WORD    wVersionRequested;  
	WSADATA wsaData; 
	int     err; 

	wVersionRequested = MAKEWORD(2, 2); 
	err               = WSAStartup(wVersionRequested, &wsaData); 
	if (err != 0) {
	    fprintf(stderr, "No sockets available.\n");
	    fflush(stderr);
	    return false;
	}
#endif
    }
    return true;
}
