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
#include "../../nameserver/include/nameServCalls.h"
#include "../../dish/include/SocketComm.h"

class ServerConnection {
public  :
	ServerConnection();
    // This function will return next connection in the server connection list
	ServerConnection* GetNext(void);
	// This function will return current connection service name
	char* GetService(void);
    // This function will evaluate Access command on the connected server and
	// will return the results.
	char* EvalCommand(char* command);
    // This function will add the new connection to the connections list.
	// it will query name server for the given service and it will open the socket
	// to the server.
    static ServerConnection* AddNewConnection(char* service);
	// This function will search the connections list for the given service name.
	// It will return first connection with matching service name or NULL if not
	// found.
    static ServerConnection* FindConnection(char* service);
	// This function will try to locate the connection with the given service name 
	// and will return it if found. It will try to create new connection if no
	// connection with the given service name is available.
    static ServerConnection* EstablishConnection(char* service);
    static int RemoveConnection(char* service);

    static ServerConnection *connections_list;
    static ServerConnection *connections_list_last;
private:
	// This function will try to connect to the server with the given service name 
	// and fill all internal class structures to reflect this connection.
	// Will return 1 if OK.
	int Connect(char* service);

	char*       m_Service;     // Model name to which this class is connected
	int         m_ConnectionId;  // This connection id
	sockaddr_in m_Server;        // Server address and port
	int         m_Socket;        // Socket to the model server

	ServerConnection *m_Next;
};
