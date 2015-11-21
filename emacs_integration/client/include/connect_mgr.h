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
// ******************************************************************************************
typedef struct {
	int m_IntegratorAddress;
	int m_IntegratorPort;
	int m_IntegratorSocket;

	int m_DriverAddress;
	int m_DriverPort;
	int m_DriverListeningSocket;
} CConnectionDescriptor;

//############################################################################################
// This class represents the client record in the clients linked list.
// Any client record contain client name, client TCP address and port and server TCP 
// address and port.
//############################################################################################
class ClientDescriptor {
public:
    ClientDescriptor(int id, 
		             int client_socket,
		             int client_port,
		             int client_tcp_addr);

    ~ClientDescriptor();
    static void             AddNewClient(ClientDescriptor *sd);
    static void             RemoveClient(int id);
    static void             RemoveClient(ClientDescriptor *p);
    static void             RemoveAllClients(void);
    static ClientDescriptor *LookupID(int id);
    int   m_ID;
    int   m_ClientSocket;
    int   m_ClientPort;
    int   m_ClientTCP;
    ClientDescriptor *m_Next;
    static ClientDescriptor *sd_list;
    static ClientDescriptor *sd_list_last;
};

// ------ Routines that are exported to the driver  ---------------
void CreateNewClient(int id, int client_socket, int client_port, int client_tcp);
int CreateListener(void);
int ConnectToIntegrator(int& integrator_port, int& integratoraddr);
int CheckSocket(int socket);
int MakeSocketsArray(int server, int integrator, int *sockets);
int WaitSocket(int *sockets, int amount);
