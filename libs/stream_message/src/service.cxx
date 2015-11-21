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
#include "service.h"
#include "startprocess.h"
#include "nameServCalls.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef ISO_CPP_HEADERS
#include <fstream>
#include <sstream>
#else
#include <fstream.h>
#include <strstream.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#include <ws2tcpip.h> // for socklen_t
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
#define closesocket(sock) close(sock)
#endif


#include "transport.h"
using namespace nTransport;
using namespace MBDriver;
namespace std {};
using namespace std;

// ===========================================================================
//
ServiceName::ServiceName(const char *nm)
{
    size_t len = strlen(nm);
    name = new char[len+1];
    strcpy(name, nm);
}

// ===========================================================================
//
bool ServiceName::getPortAndAddr(int &port_param, int &IP_addr,
				 string **diagnostic) const
{
    int ret;
    if (!initialize()) {
	if (diagnostic != NULL) {
	    *diagnostic = new string("failed to initialize for sockets");
	}
	return false;
    }
    else if (name == NULL) {
	if (diagnostic != NULL) {
	    *diagnostic = new string("null service name");
	}

	return false;
    }
    else {
#ifdef DEBUG19953
	fprintf(stderr, "getPortAndAddr for %s\n", name);
	fflush(stderr);
#endif
        if ((ret = NameServerGetService(name, port_param, IP_addr)) != 1) {
	    if (diagnostic != NULL) {
		if (ret != 0) {
		    *diagnostic = new string("failed name server connection in NameServerGetService");
		}
		else {
		    string diag = "failed NameServerGetService: Name server did not find service ";
		    diag.append(name);
		    diag.append(".");
		    *diagnostic = new string(diag);
		}
	    }
	    return false;
	}
    }
    return true;
}

// ===========================================================================
//
const char *ServiceName::getName() const
{
    return name;
}

// ===========================================================================
//
ServiceName::~ServiceName()
{
    delete [] name;
}

// ===========================================================================
// This function will start the name_server.  Returns false to indicate 
// failure, true to indicate that process was started.
//
static bool start_name_server() {
    
    char *p = getenv("PSETHOME");
#ifdef ISO_CPP_HEADERS
    stringstream cmd;
#else
    strstream cmd;
#endif
    if(p != NULL) {
	cmd << p << "/bin/";
    }
    cmd << "nameserv";
#ifndef ISO_CPP_HEADERS
    cmd << ends;
#endif
    string cmd_str = cmd.str();

    char *argv[] = { NULL };
#ifdef DEBUG19953
    fprintf(stderr, "ServiceName starting name server.");
    fflush(stderr);
#endif
    return start_process(cmd_str.c_str(), argv, 1) != 0;
}

// ===========================================================================
// Register this service on the given listening socket.
// -- code borrowed from paraset/src/serverspy/serverspy.cpp --
//
bool ServiceName::doRegister(int socket) const
{
    // First, make sure that nameserv is up and accepting connections
    if (!initialize()) {
	return false;
    }
#ifdef DEBUG19953
    fprintf(stderr, "doRegister contacting name server.\n");
    fflush(stderr);
#endif
    int test_sock = ConnectToNameServer();
    if (test_sock < 0) {
	// Couldn't connect, so try to start nameserv
	if (start_name_server() != 0) {
	    // Wait for nameserv to start accepting connections
	    for (int count = 0; count < 3000; count++) {
		test_sock = ConnectToNameServer();
		if (test_sock >= 0) {
		    break;
		}
	    }
	}
    }

    if (test_sock >= 0) {
	closesocket (test_sock);
    } else {
	fprintf(stderr, "Unable to contact or start nameserver.  Please run nameserv by hand.\n");
	fflush(stderr);
	return false;
    }

    // Next, register service
    struct sockaddr_in sock_info;
    socklen_t len = sizeof(sock_info);
    int res = -1;
    int getsockres = getsockname(socket, (struct sockaddr *)&sock_info, &len);
    if(getsockres == 0) {
#ifdef DEBUG19953
	fprintf(stderr, "doRegister calling NameServerRegisterService for %s.\n", name);
	fflush(stderr);
#endif
	res = NameServerRegisterService(name, ntohs(sock_info.sin_port));
    }
    if (res == 1) {
	// fprintf(stderr, "Successfully registered %s service.\n", name); fflush(stderr);
    } else {
	fprintf(stderr, "Unable to register %s service: getsockname code %d, register code %d.\n", name, getsockres, res);
	fflush(stderr);
	return false;
    }
    return true;
}

// ===========================================================================
// Register this service on the given listening socket.
//
void ServiceName::unregister() const
{
    NameServerUnRegisterService(name);
}

// ===========================================================================
// Construct a ServicePort object based on a service name.
// Queries the name server to resolve the service.
//
ServicePort::ServicePort(const char *name)
  : isGood(false), whyBad(NULL)
{
    ServiceName nm(name);
    isGood = nm.getPortAndAddr(port, IPAddr, &whyBad);
}

// ===========================================================================
//
ServicePort::ServicePort(int port, int IP_addr)
: port(port),
  IPAddr(IP_addr),
  isGood(true)
{
}

// ===========================================================================
// Get the port and IP address.
//
bool ServicePort::getPortAndAddr(int &port_param, int &IP_addr, string **diagnostic) const
{
    if (isGood) {
	port_param = port;
	IP_addr = IPAddr;
    }
    if (whyBad == NULL) {
        if (diagnostic != NULL)
	    *diagnostic = NULL;
    }
    else {
        if (diagnostic != NULL)
	   *diagnostic = new string(*whyBad);
    }
    return isGood;
}
