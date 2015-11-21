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
// gui_message_manager.cxx


#include <fstream>
#include <sstream>
#include <cassert>

#include <algorithm>
#ifndef MSVC_NOTBROKEN
#define max _MAX
#endif

#include <limits>
#include "gui_message_manager.h"
#include "simple_msgstream.h"
#include "../gui/msg_gui.h"
#include "../parsemake/setech_utils.h"


// globals:
static message_manager *the_message_manager = 0;

// the dispatcher:

message_manager::message_manager(char const *dbfname) :
	defaultmsg("UNKNOWN MESSAGE: $. ARGUMENTS: ", 1, 1)
{
	readmsgdb(dbfname);
}

message_manager::~message_manager()
{
}


message_stream &
message_manager::msgid(string const &m)
{
	// lookup key
	bool msg_not_found = false;
	messages_type::iterator i = messages.find(m);
	message *msgp;
	if (i != messages.end()) {
		msgp = &i->second;
	}
	else {
		msg_not_found = true;
		msgp = &defaultmsg;
	}
	message::errorstatus = max(message::errorstatus, msgp->errorcode);
	message_stream *msp;
	if (gui_available) {
		msp = new gui_message_stream(*msgp, new stringstream);
	}
	else {	// queue until available
		if (!errorqueue) {
			errorqueue = new queue<pair<message *, stringstream *> >;
		}
		stringstream *ssp = new stringstream;
		msp = new simple_message_stream(*msgp, ssp->rdbuf());
		errorqueue->push(pair<message *, stringstream *>(msgp, ssp));
	}
	if (msg_not_found) {
		*msp << m << eoarg;
	}
	return *msp;
}




void
message_manager::readmsgdb(char const *fname)
{
	// open file
	ifstream input(fname, ios_base::in);

	// read all lines
	stringstream s;
	while (input.get(*s.rdbuf()) && input.good() ) {
		// consume newline:
		input.ignore();
		// add to index if appropriate
		if (s.peek() != '#') { // exclude comments beginning with #
			string key;
			getline(s, key, '|');
			string text;
			getline(s, text, '|');
			int severity;
			s >> severity;
			s.ignore(numeric_limits<int>::max(), '|');
			int errorcode;
			s >> errorcode;

			// make sure this message hasn't already been seen:
			if (messages.find(key) == messages.end()) {
				messages.insert(messages_type::value_type(key, message(text.c_str(), severity, errorcode)));
			}
		}
		// wipe stream for next line:
		s.str("");
		s.clear();
	}

	// close
	input.close();
}

// message_stream base class:
ostream &
msgid(msg_key key)
{
	ostream *ret;
	if (!the_message_manager) {
		if (!message_manager::manager_shutdown) {
			string msgfile(setech_utils::psethome());
			msgfile += "\\lib\\new_messages.dat";
			the_message_manager = new message_manager(msgfile.c_str());
			ret = &the_message_manager->msgid(string(key));
		}
		else {
			ret = &cerr;
		}
	}
	else {
		ret = &the_message_manager->msgid(string(key));
	}
	return *ret;
}

bool message_manager::gui_available = false;

typedef queue<pair<message *, stringstream *> > errorqueue_type;
errorqueue_type *message_manager::errorqueue = 0;

void
message_manager::notify_gui_available()
{
	gui_available = true;
	if (errorqueue) {
		while (!errorqueue->empty()) {
			// these are simple string streams, not terminated:
			stringstream *sp = errorqueue->front().second;
			*sp << (char)0;
			display_message_dialog(sp->str().c_str());
			delete(sp);
			errorqueue->pop();
		}
	}
}

bool
message_manager::manager_shutdown = false;

void
message_manager::shutdown()
{
	if (the_message_manager) {
		delete the_message_manager;
		the_message_manager == 0;
		manager_shutdown = true;
	}
}
