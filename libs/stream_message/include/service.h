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
#ifndef TCP_SERVICE_H
#define TCP_SERVICE_H

#include <stdlib.h>
#include <string>
namespace std {};
using namespace std;

namespace nTransport {

// ===========================================================================
// For services negotiated by the name server.
// The state of a ServiceName object simply holds onto the service name.    
//
class ServiceName {
public:
    ServiceName(const char *name);
    const char *getName() const;
    ~ServiceName();

    // Get the port and IP address from the name server.
    bool getPortAndAddr(int &port, int &IP_addr, string **diagnostic = NULL) const;

    // Register a newly-available service with the name server.
    bool doRegister(int listening_socket) const;

    // Unregister a service with the name server.
    void unregister() const;
private:
    char *name;
};

// ===========================================================================
// For TCP services.
// The state of a ServicePort object simply holds onto the port and IP address.    
//
class ServicePort {
public:
    ServicePort(const char *name);
    ServicePort(int port, int IP_addr);

    // Get the port and IP address.
    bool getPortAndAddr(int &port, int &IP_addr, string **diagnostic = NULL) const;
private:
    int port;
    int IPAddr;
    bool isGood;
    string *whyBad;
};
}

#endif // TCP_SERVICE_H
