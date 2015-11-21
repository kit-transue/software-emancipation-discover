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
#ifndef _MSG_COMMS_H
#define _MSG_COMMS_H

#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#ifdef ISO_CPP_HEADERS
#include <sstream>
#else
#include <strstream.h>
#endif
#include <string>
#include <vector>
#include <stack>
#include <map>
namespace std {};
using namespace std;

#include "transport.h"
#include "msg_format.h"

//Derived class from msg_class. 
//Translates message to xml format and write it on the socket 
class msg_socket : public msg_class {
public:
  ostream& received_eoarg(ostream&);
  void received_eom(ostream&);
  ostream* received_msg();

  msg_socket(const char* msg_id, msg_sev severity);
  msg_socket(const char* msg_id, const char *format_str, msg_sev severity);
  msg_socket();
  ~msg_socket();
private:
  ::MBDriver::Message* create_xml_object();
  void initialize_xml();
  ostream* create_new_stream();

  //member data
  vector<ostream*> arg_streams;
};

namespace nTransport {
    class ServicePort;
}
using namespace nTransport;

class AutoMessageGroup_manager
{
public:
	AutoMessageGroup_manager();
	void report_new_group(char const *name);
	void push(unsigned long);
	void pop();
	void init(const ServicePort &sp, char const *message_group_id);
	char const *current();
private:
	// server may give us blocks of msgids:
	unsigned long msgid_end;  // last valid msgid
	unsigned long msgid_cur;  // next available id
	
	stack<unsigned long> id_stack;
	ServicePort *servicePort;
};


#endif /* _MSG_COMMS_H */
