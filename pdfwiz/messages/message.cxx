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
// message.cxx
// 3.mar.98 kit transue


#include "message.h"
#include <cassert>
#include <iostream>
#include <string>
#include <limits>
#include <cstring>

using namespace std;


int message::errorstatus = 0;

// the inserters:

struct msg_inserter_eom eom;
struct msg_inserter_eoarg eoarg;
void
operator<< (ostream & os, msg_inserter_eom &)
{
	message_stream *msp = dynamic_cast<message_stream *>(&os);
	assert(msp);
	if (msp) {
		msp->internal_eom();
	}
}

ostream &
operator<< (ostream & os, msg_inserter_eoarg &)
{
	ostream *osp = &os;
	message_stream *msp = dynamic_cast<message_stream *>(osp);
	assert(msp);
	if (msp) {
		osp = &msp->eoarg();
	}
	return *osp;
}


// message_stream implementation
// construct/copy/destroy
message_stream::message_stream(message const &m, streambuf * sb) : ostream(sb),
	themessage(m)
{
}

message_stream::~message_stream()
{
}


// accessors:
message const &
message_stream::msg()
{
	return themessage;
}

void
message_stream::internal_eom()
{
	// create temporary that will destroy this when destroyed:
	// (required for housekeeping in case exception is thrown)
	// could be conditional on self-destruct flag:
	struct message_stream_cleaner {
		message_stream *msp;
		message_stream_cleaner(message_stream *m) : msp(m) {};
		~message_stream_cleaner() {delete msp;};
	} cleaner(this);

	eom();
	// if we haven't unwound because of an exception, cleaner deletes now:
	// could return value if desired
}


// the message:
message::message(string const &s, int sv, int ec) :
	errorcode(ec),
	severity(sv),
	args(0)
{
	// look for $position indicators
	string::size_type start = 0;
	string::size_type pos;
	while (start != string::npos) {
		pos = s.find('$', start);
		texts.push_back(s.substr(start, pos - start));
		if (pos != string::npos) {
			// skip over the $:
			++pos;
			argmap.push_back(args);
			++args;
		}
		start = pos;
	}
}
	
