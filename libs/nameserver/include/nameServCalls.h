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
#ifndef _NAMESERVCALLS_H
#define _NAMESERVCALLS_H

#define NS_PORT 28948
#define BROADCAST_SERVER_PORT 28946

#define REGISTER_SERVER   0
#define LIST_OF_SERVERS   1
#define FIND_SERVER       2
#define UNREGISTER_SERVER 3
#define STOP              4 

class NameServerServiceList {
public:
    NameServerServiceList(char const *nm) { name = strdup(nm); next = NULL; }
    ~NameServerServiceList() { free(name); if(next) delete next; }
    
    NameServerServiceList *getNext(void) { return next; }
    char const            *getName(void) { return name; }

    char                  *name;
    NameServerServiceList *next;
};

// Function connects to name server.
// Returns -1 on failure
// Returns socket if successful
int ConnectToNameServer(void);
int ConnectToNameServerRemote(char const* host);

// Function connects to name server and asks it for specified service
// returns 1 on success and sets up "service_port" and "tcp_addr"
// returns 0 if no service found
// returns -1 if unable to connect to name server
int NameServerGetService(char const *name, int &service_port, int &tcp_addr);
int NameServerGetServiceRemote(char const *host, char const *name, int &service_port, int &tcp_addr);

// Function connects to name server and fills up list of services
// returns 1 on success and "services" gets initialized to list of services
// returns 0 if no service found
// returns -1 if unable to connect to name server
int NameServerGetListOfServices(NameServerServiceList **services);
int NameServerGetListOfServicesRemote(char const* host, NameServerServiceList **services);

// Function connects to name server and registers specified service
// name - service name
// port - service port
// returns 1 on success
// returns -1 if unable to connect to name server
int NameServerRegisterService(char const *name, int port);

// Function connects to name server and unregisters specified service
// name - service name
// returns 1 on success
// returns -1 if unable to connect to name server
int NameServerUnRegisterService(char const *name);

// Some internal functions
int SendCommand(int socket, unsigned char cmd);
int ReceiveCommand(int socket, unsigned char *cmd);
int SendBuffer(int socket, char const *buf, int len);
int SendInteger(int socket, int val);
int ReceiveBuffer(int socket, char *buf, int len);
int ReceiveInteger(int socket, int *val);
int SendString(int socket, char const *str);
char *ReceiveString(int socket);
int GetNameServerPort(void);


#endif

