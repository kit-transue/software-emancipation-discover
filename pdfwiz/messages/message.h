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
// message.h
// 3.mar.98 kit transue
// stream based, database driven error messages

// syntax: msg(msgid) << [stream formattable data] << eoarg << 
// [more << stuff] << eom;
// msgid is index into database that specifies severity, help context,
// order of argument output, additional text, exit code
// syntax of message database:  msgid|cannot open $2: OS error $1|...

// eom can post a dialog, possibly with option to continue/[throw]
// the message_stream isthread-safe, meaning all state information is
// encapsulated in the stream object returned from the manager.

#if !defined(_MESSAGE_H_)
#define _MESSAGE_H_


#pragma warning(disable:4786)

#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;


typedef char const *msg_key;

// the messages:
class message
{
public:
	// class globals:
	static int errorstatus;

	// construct/copy/destroy:
	message(string const &s, int severity, int errorcode);

	// members:
	vector<string> texts;
	int args;
	int errorcode;
	int severity;
	vector<int> argmap;
};


// inserters:
extern struct msg_inserter_eom{
} eom;

extern struct msg_inserter_eoarg{
} eoarg;

// inserters:
// void so you cannot send more stuff to the closed message:
// Note: could be int or something to convey user preference (dialog: OK/cancel)
void
operator<< (ostream &, msg_inserter_eom &);

ostream &
operator<< (ostream &, msg_inserter_eoarg &);


class message_stream : public ostream
{
public:
	// construct/copy/destroy
	message_stream(message const &, streambuf *);
	virtual ~message_stream();

	// actions:
	virtual void eom() = 0;
	virtual ostream &eoarg() = 0;

	// called by inserter:
	void internal_eom();

	// accessors:
	message const &msg();
private:
	message const &themessage;
};

// the dispatcher:
ostream &msgid(msg_key);	// message identifier

#endif // !defined(_MESSAGE_H_)
