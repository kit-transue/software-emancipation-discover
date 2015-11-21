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
#ifndef TCP_INTERFACES_H
#define TCP_INTERFACES_H

// ===========================================================================
// This header file is separate from transport.h only for the sake of easy
// template instantiation.  Code which uses Transport should include transport.h.
//

#ifdef ISO_CPP_HEADERS
#include <cstddef>
namespace std {}
using namespace std;
#else
#include <stddef.h>
#endif

namespace nTransport {

template<typename T> class Serializer;

// ===========================================================================
// Connection is an abstract class, whose objects are supplied by the
// transport module.
// It supports sending and receiving messages across an unreliable network.
// Send and receive each have the possibility of blocking. 
// They will continue to block, but if the connection fails will
// automatically return.  In case of failure, send returns false,
// and receive will return null pointer.
//
// If the service entails bidirectional communications, send and receive must
// be called with care to coordinate sending and receiving at the times
// when the opposite end of the pipe performs the complementary
// operation.  Simultaneous sends/receives in distinct threads are also possible.
//
// IsError can be called after a send/receive fails. The peer at the 
// other end of the connection may simply stop communicating,
// a normal situation for which isError will return false.
//
template<typename T>
class Connection {
public:
    virtual bool send(const T&) = 0;
    virtual T *receive() = 0; // Caller does not free the returned object.
    virtual bool isError() const = 0;
    virtual ~Connection() {}
};

// ===========================================================================
class Listener {
public:
    virtual ~Listener() {}
    virtual void shut() = 0;
};

// ===========================================================================
// Communicator is an abstract class template, whose objects are supplied
// by the user of this transport module.  If 'listen' is called for a
// Communicator, then all of these member functions may be called
// from server threads and must coordinate access to global resources
// (including 'this' Communicator) using interprocess control such as JTC.
// If 'connect' is called, then these member functions are called
// in the same thread as connect.
//
template<typename T>
class Communicator {
public:
    virtual void communicate(Connection<T> &) = 0;

    // If this Communicator is used with 'listen',
    // called when listening socket is ready, prior to accepting connections.
    virtual void ready(Listener &shut_object) {}

    // The caller of getSerializer is responsible for calling
    // releaseSerializer when the Serializer is no longer needed.
    virtual Serializer<T> *getSerializer() = 0;
    virtual void releaseSerializer(Serializer<T> *) = 0;

    // The caller of serialize uses the data string immediately,
    // and does not delete it.
    virtual ~Communicator() {};
};

// ===========================================================================
// Serializer is an abstract class template, whose objects are manufactured by
// the client or server Communicator.  It parses the received stream of bytes
// progressively.  When Connection::receive is called, and bytes are available,
// it calls 'add' for each one, and a T object will be returned when complete.
// Note that to make this work, serialization and parsing must use an encoding
// which indicates the end of the object unambigously.
//
// A lot of work gets pushed onto the parser.
//     o   uncertainty regarding how many bytes to read
//     o   responding to bugs in the serialized data, e.g. by
//         returning an object which indicates an error
//     o   extra difficulty if the connection handles more than
//         one kind of object
//     o   remembering the effect of preceding calls to 'add' when
//         dealing with the current call to 'add', and freeing up
//         any storage used in the remembering
// 
// The Serializer object tracks and frees storage for the buffer load of data
// returned by 'serialize'.  The buffer remains valid only until the next
// call to 'serialize' or until the Serializer is destroyed.
// Similarly, the Serializer tracks and frees storage for the object returned
// by 'add', which remains valid until the next call to 'add'.
//
template<typename T>
class Serializer {
public:
    virtual void reset() = 0; // Prepare to parse a new chunk.
    virtual T *add(const char *, size_t) = 0;
    virtual size_t require() = 0; // num bytes to ask for
    virtual void serialize(const T &, const char *&data, size_t &length) = 0;
    virtual ~Serializer() {}
};

}
#endif // TCP_INTERFACES_H
