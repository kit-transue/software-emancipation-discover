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
#ifndef _MSG_FORMAT_H
#define _MSG_FORMAT_H

#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#ifdef ISO_CPP_HEADERS
#include <fstream>
#include <sstream>
#else
#include <fstream.h>
#include <strstream.h>
#endif /* ISO_CPP_HEADERS */

#include <string>
#include <vector>
#include <map>
#include <set>
namespace std {};
using namespace std;

extern char const *sev_str[];

typedef enum {
	popup,
	status,
	standard
} msg_disposition;

class msg_class;
class ostream_pool {
public:
  bool add_stream_to_pool(ostream* out, msg_class* mc_ptr);
  bool remove_stream_from_pool(ostream* out);
  msg_class* find_stream_in_pool(ostream* out);
  ostream_pool() {};
  ~ostream_pool() {};
private:
  map<ostream*, msg_class*> ostream_mc_map;
};

ostream& operator<<(ostream& out, msg_class& mc);

//Base class for messages
class msg_class {
public:
  //pure virtual functions, have to be defined by derived class
  virtual ostream& received_eoarg(ostream&) = 0;
  virtual void received_eom(ostream&) = 0;
  virtual ostream* received_msg() = 0;

  msg_disposition get_disposition() { return disposition; }
  void set_disposition(msg_disposition d) { disposition = d; }
  bool get_verbose_flag() { return verbose_flag; }
  void set_verbose_flag(bool val) { verbose_flag = val; }
  const string& get_message_id() { return message_id; }
  const string& get_format_string() { return format_string; }
  msg_sev get_original_severity() const { return original_severity; }
  msg_class(const char* msg_id, const char *format_str, msg_sev);
  msg_class(const char* msg_id, msg_sev);
  msg_class(const char* msg_id);
  msg_class();
  virtual ~msg_class();
private:
  //member data
  msg_disposition disposition;
  bool verbose_flag;
  string message_id;
  string format_string;
  msg_sev original_severity;
};

//Derived class from msg_class.
//Subsitutes arguments in the message and write it on the stdout
class msg_ostream : public msg_class {
public:
  ostream& received_eoarg(ostream&);
  void received_eom(ostream&);
  ostream* received_msg();

  msg_ostream(const char* msg_id, ostream &destination, msg_sev sev);
  msg_ostream(const char* msg_id, const char *format_str, ostream &destination, msg_sev sev);
  ~msg_ostream();

  //public member data
#ifdef ISO_CPP_HEADERS
  ostringstream strm_out;
#else
  ostrstream strm_out;
#endif
private:
  ostream& flush_rest_msg_arr(ostream&);
  string get_next_msg_from_arr();
  int parse_message();

  //private member data
  int args_cnt;
  int next_in_queue;
  vector<string> msg_arr;
  ostream &destination_stream;
};

struct msg_traits {
  msg_disposition disposition;
  bool verbose;
  string format_str;
};

bool search_msg_database(char const *, msg_traits &);

#endif /* _MSG_FORMAT_H */

