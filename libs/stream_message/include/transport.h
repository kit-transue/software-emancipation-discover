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
#ifndef TCP_TRANSPORT_H
#define TCP_TRANSPORT_H

// ===========================================================================
// Declares all interfaces for the Transport namespace and module.
// 
// The essential features of this module are:
//      class Connection           in tptinterfaces.h
//      class Communicator         in tptinterfaces.h
//      class Serializer           in tptinterfaces.h
//      template function listen   below
//      template function connect  below
//      function shut              below
// Due to the necessities of template instantiation, this file is cluttered
// with implementation details. The following should be ignored:
//      class Transport
//      function listen
//      function connect
//      function initialize
//

#include "tptinterfaces.h"
#include "aconnectn.h"
#ifdef TRANSPORT_SERVER
#include "srvthread.h"
#endif
#include <string>
namespace std {};
using namespace std;

namespace nTransport {

class ServiceName;
class ServicePort;

template<typename T>
class Transport {
public:
    static void clientCommunicate(void *clientp, int connected_sock, void *connectionp)
    {
	Communicator<T> &client = *(Communicator<T> *)clientp;
	if (connectionp == NULL) {
	    ActualConnection<T> conn(client, connected_sock);
	    client.communicate(conn);
	}
	else {
	    // Uses client only for serializer. Return the connection.
	    *(ActualConnection<T> **)connectionp = new ActualConnection<T>(client, connected_sock);
	}
    }
#ifdef TRANSPORT_SERVER
    static void startServer(int connected_socket, void *serverp) {
	Communicator<T> &server = *(Communicator<T> *)serverp;
	SAThreads::Thread *h = new SAThreads::Thread("transport server");
    	ServerThread<T> *connected_thread
		= new ServerThread<T>(connected_socket, server, h);
	h->start(*connected_thread);
    }
#endif
    static void ready(void *serverp, Listener &listener) {
	Communicator<T> &server = *(Communicator<T> *)serverp;
	server.ready(listener);
    }
};

extern bool connect_generic(const ServicePort &service, void *clientp, void (*)(void *, int, void *),
		            string **, void *connectionp);
#ifdef TRANSPORT_SERVER
extern bool listen(const ServiceName &, void *serverp, void (*)(int, void *),
		   void (*)(void *, Listener &), string **);
#endif
extern bool initialize();

// ===========================================================================
// Essential connections for transporting objects.
//
// ===========================================================================
// Connect a client to a service.  Return false if could not connect.
//
// If the caller passes a non-null diagnostic, the string pointer is set to
// either NULL or to a pointer to a suitable error message.  It is up to the
// caller to free this pointer. 
//
// If the caller passes a non-null connection, the Connection pointer is set
// to either NULL or to a pointer to an object which should be deleted, and
// there's no callback to 'communicate'.
//
template<typename T>
bool connect(const ServicePort &service, Communicator<T> &client,
	     string **diagnostic = NULL, Connection<T> **connection = NULL)
{
    return connect_generic(service, &client,
		           &Transport<T>::clientCommunicate,
		           diagnostic, connection)
	 && (connection == NULL || *connection != NULL);
}

#ifdef TRANSPORT_SERVER
// ===========================================================================
// Start a service.  Since multiple connections are handled by JTC's threads,
// if 'listen' is called, JTC must be intialized with
//          JTCInitialize initialize;
// Return false if could not set up service correctly.  Return true if
// service shut down normally, such as when 'server' invokes 'shut'.  Note
// that 'listen' may return while a connection thread is still active, so
// the destruction of 'server' within the listening thread must be avoided.
//
// The diagnostic is as for 'connect' above.
//
template<typename T>
bool listen(const ServiceName &service, Communicator<T> &server,
	    string **diagnostic = NULL)
{
    return listen(service, &server, &Transport<T>::startServer,
                                    &Transport<T>::ready,
				    diagnostic);
}
#endif

// ===========================================================================
// A communicator which sends a single object.
// Its use of a single serializer makes it unsuitable for use with 
// multiple threads.
//
template<typename T>
class SingleSender : public Communicator<T> {
public:
    SingleSender(T &o, Serializer<T> &s) : obj(o), serializer(s), hadSuccess(false) {}
    void communicate(Connection<T> &conn) { hadSuccess = conn.send(obj); }
    Serializer<T> *getSerializer() { return &serializer; }
    void releaseSerializer(Serializer<T> *serializer) {}
    bool success() { return hadSuccess; }
private:
    T& obj;
    bool hadSuccess;
    Serializer<T> &serializer;
};

// ===========================================================================
// A communicator which performs one query for response.
// The response is stored into the pointer given to the constructor,
// and it is up to the caller to free the response object.
// Its use of a single serializer makes it unsuitable for use with 
// multiple threads.
//
template<typename T>
class QuerySender : public Communicator<T> {
public:
    QuerySender(T &q, T *&r, Serializer<T> &s) : query(q), response(r), serializer(s) {}
    void communicate(Connection<T> &conn)
    { 
	response = conn.send(query) ? conn.receive() : NULL;
    }
    Serializer<T> *getSerializer() { return &serializer; }
    void releaseSerializer(Serializer<T> *serializer) {}
private:
    T& query;
    T *&response;
    Serializer<T> &serializer;
};

// ===========================================================================
// A communicator which sends nothing.
// Its use of a single serializer makes it unsuitable for use with 
// multiple threads.
//
template<typename T>
class NonSender : public Communicator<T> {
public:
    NonSender(Serializer<T> &s) : serializer(s) {}
    void communicate(Connection<T> &conn) {}
    Serializer<T> *getSerializer() { return &serializer; }
    void releaseSerializer(Serializer<T> *serializer) {}
    bool success() { return true; }
private:
    Serializer<T> &serializer;
};

}
#endif // TCP_TRANSPORT_H
