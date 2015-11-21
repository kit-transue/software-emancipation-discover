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
// ===========================================================================
// private header, solely for implementation of transport module
// ===========================================================================

#ifndef TCP_ACONNECTION_H
#define TCP_ACONNECTION_H

#include <stdlib.h>
#include "tptinterfaces.h"

namespace nTransport {

extern int send(int, char *, int);
extern int receiveBuffer(int socket, char *buf, int len);
extern int closesock(int socket);

// ===========================================================================
// Implement the Connections which support TCP clients and servers.
// An ActualConnection object is made available when 'connect' returns
// successfully, or when accept handles a connection, so that communication
// can proceed.
//
// Send and receive may block the current thread, but if the TCP/IP
// connection fails in a detectable way, they return immediately
// indicating failure.
//
template<typename T>
class ActualConnection : public nTransport::Connection<T> {
public:
    ActualConnection(nTransport::Communicator<T> &client, int connected_socket) 
	: client(client),
	    connectedSocket(connected_socket),
	    serializer(NULL),
	    failed(false),
	    receiveBufLen(0),
	    buf(NULL),
	    didError(false)
    {}
    virtual bool send(const T &obj)
    {
	if (!failed) {
	    if (serializer == NULL) {
		serializer = client.getSerializer();
	    }
	    if (serializer != NULL) {
		const char *data;
		size_t length;
		serializer->serialize(obj, data, length);
		int success = nTransport::send(connectedSocket, (char *)data, length);
		failed = (success <= 0);
		if (failed) {
		    if (success == 0) {
			// Peer has closed the connection. (conjecture by sturner)
		    }
		    else {
			didError = true;
		    }
		}
	    }
	    else {
		failed = true;
	    }
	}
	return !failed;
    }
    virtual T *receive()
    {
	if (!failed) {
	    if (serializer == NULL) {
		serializer = client.getSerializer();
		if (serializer == NULL) {
		    failed = true;
		    return NULL;
		}
	    }
	    else {
		serializer->reset();
	    }

    	    for (;;) {
		size_t len = serializer->require();
		if (buf == NULL || receiveBufLen < len) {
		    delete [] buf;
		    buf = new char[len];
		    receiveBufLen = len;
		}
		int success = nTransport::receiveBuffer(connectedSocket, buf, len);
		if (success > 0) {
		    T *obj = serializer->add(buf, len);
		    if (obj != NULL) {
			return obj;
		    }
		}
		else {
		    if (success == 0) {
			// Peer has closed the connection.
		    }
		    else {
			didError = true;
		    }
		    failed = true;
		    return NULL;
		}
	    }
	}
	return NULL;
    }
    virtual ~ActualConnection() {
	client.releaseSerializer(serializer);
	delete [] buf;
	nTransport::closesock(connectedSocket);
    }
    virtual bool isError() const { return didError; }

private:
    nTransport::Communicator<T> &client;
    int connectedSocket;
    nTransport::Serializer<T> *serializer;
    bool failed;
    size_t receiveBufLen;
    char *buf;
    bool didError;
};

}

#endif // TCP_ACONNECTION_H
