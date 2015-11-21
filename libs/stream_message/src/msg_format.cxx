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
#ifdef _WIN32
#pragma warning(disable:4786)
#endif /* _WIN32 */

#include "msg.h"
#include "msg_format.h"

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif /* _WIN32 */

#define ARG_KEY '$'
#define BACK_SLASH '\\'

//global pool to associate ostream with msg_class
ostream_pool *stream_pool;

void (*popup_msg_callback)(msg_sev, char const *) = 0;
void (*status_msg_callback)(msg_sev, char const *) = 0;
bool generate_verbose_output = true;

char const *sev_str[] = {
	"normal",
	"warning",
	"error",
	"catastrophe",
	"unknown",
};

bool ostream_pool::add_stream_to_pool(ostream* out, msg_class* mc_ptr)
{
  bool ret_val = true;
  map<ostream*, msg_class*>::iterator it;
  it = ostream_mc_map.find(out);
  if ( it != ostream_mc_map.end()) {
	cerr << "ostream exists in the pool for message id: " << mc_ptr->get_message_id() << endl;
	ret_val = false;
  }
  else {
	ostream_mc_map.insert(pair<ostream *const, msg_class*>(out, mc_ptr));
  }
  return ret_val;
}

msg_class* ostream_pool::find_stream_in_pool(ostream* out)
{
  msg_class *mc_ptr = 0;
  map<ostream*, msg_class*>::iterator it;
  it = ostream_mc_map.find(out);
  if ( it != ostream_mc_map.end()) {
	mc_ptr = it->second;
  }
  else {
	cerr << "Could not find ostream in the pool.\n";
  }
  return mc_ptr;
}

//remove stream->msg_class entry from pool
//returns true when it suceeds and false when it fails 
bool ostream_pool::remove_stream_from_pool(ostream* out)
{
  bool ret_val = false;

  map<ostream*, msg_class*>::iterator it;
  it = ostream_mc_map.find(out);
  if (it != ostream_mc_map.end()) { 
    ostream_mc_map.erase(it);
	ret_val = true;
  }

  return ret_val;
}

//received eom (end of message), last item on a message line
void operator<<(ostream& out, endOfMessage& eom)
{
  msg_class *mc_ptr = stream_pool->find_stream_in_pool(&out);
  if (mc_ptr) {
    mc_ptr->received_eom(out);

    //cleanup all data structures related to this message when eom is received
    stream_pool->remove_stream_from_pool(&out);
    delete mc_ptr;
  }
  return;
}

//received eoarg (end of argument)
ostream& operator<<(ostream& out, endOfArg& eoarg)
{
  ostream* new_out = 0;
  msg_class *mc_ptr = stream_pool->find_stream_in_pool(&out);
  if (mc_ptr) {
    new_out = &mc_ptr->received_eoarg(out);
  }
  return (new_out) ? *new_out : out;
}


//msg_class default constructor
msg_class::msg_class() :
	disposition(standard),
	verbose_flag(false),
	message_id(""),
	format_string(""),
	original_severity(unknown_sev)
{
}

//msg_class constructor
msg_class::msg_class(const char* msg_id) :
	disposition(standard),
	verbose_flag(false),
	message_id(msg_id),
	format_string(msg_id),
	original_severity(unknown_sev)
{
}

msg_class::msg_class(const char* msg_id, msg_sev severity) :
	disposition(standard),
	verbose_flag(false),
	message_id(msg_id),
	format_string(msg_id),
	original_severity(severity)
{
}

msg_class::msg_class(const char* msg_id, const char *format_str, msg_sev severity) :
	disposition(standard),
	verbose_flag(false),
	message_id(msg_id),
	format_string(format_str),
	original_severity(severity)
{
}

//msg_class destructor
msg_class::~msg_class() 
{
}


/******************************************
Member functions for class msg_ostream
*******************************************/

//When eoarg is received write next message in queue
ostream& msg_ostream::received_eoarg(ostream& out)
{
  out << get_next_msg_from_arr();
  args_cnt--;
  return out;
}

void msg_ostream::received_eom(ostream& out)
{
  flush_rest_msg_arr(out);

#ifndef ISO_CPP_HEADERS
  strm_out << ends;
#endif

  // Generate the message unless verbose is off and the message is verbose
  if (!get_verbose_flag() || generate_verbose_output) {
    bool send_to_destination_stream = true;
    string m = strm_out.str();

    //if this message is flagged to be popped do it now
    switch(get_disposition()) {
    case popup:
      if (popup_msg_callback)
	popup_msg_callback(get_original_severity(), m.c_str());
      break;
    case status:
      if (status_msg_callback) {
	status_msg_callback(get_original_severity(), m.c_str());
	send_to_destination_stream = false;
      }
      break;
    }

    if (send_to_destination_stream)
      destination_stream << m << endl;
  }

  return;
}

ostream* msg_ostream::received_msg()
{
  //initialize stream_pool
  if (!stream_pool)
    stream_pool = new ostream_pool;

  //add stream and msg_class to pool 
  stream_pool->add_stream_to_pool(&strm_out, this);
  return &strm_out;
}

ostream& msg_ostream::flush_rest_msg_arr(ostream& out)
{
  //print the last part of message id
  if (next_in_queue < msg_arr.size()) {
    out << get_next_msg_from_arr();
  }
  if (args_cnt > 1 || next_in_queue < msg_arr.size()) {
    cerr << "Wrong number of arguments for message id: " << get_message_id() << endl;
	while (next_in_queue < msg_arr.size())
	  out << get_next_msg_from_arr();
  }
  return out;
}

//return the next message in queue and increment next_in_queue variable
string msg_ostream::get_next_msg_from_arr()
{
  return (next_in_queue >= 0 && next_in_queue < msg_arr.size() ? (string)(msg_arr[next_in_queue++]) : (string)"");
}

//This function parses the text passed to "msg" function and breaks it up into
//an array of message components separated by argument(s)
int msg_ostream::parse_message()
{
  int ret_val = 0, st_ind = 0, ind = 0, found = -1;
  const string& message = get_format_string();
  do {
    found = message.find(ARG_KEY, ind);
	ind = (found >= 0) ? found : message.size();
	if (!ind || message.at(ind-1) != BACK_SLASH ) {
	  //add this component of message to msg_arr and increment args_cnt
	  msg_arr.push_back(message.substr(st_ind, ind-st_ind));
	  //incr the argument counter
	  if (found >= 0 ) args_cnt++;
	  if ( ind < message.size()) {
		//while char is number increment ind, to skip argument number that follows $
		do {
		  ind++;
		  if ( ind >= message.size() )
			break;
		} while (isdigit(message.at(ind)));
	  }
	  st_ind = ind;
	}
	else {
      if ( ind == message.size() -1 ) {
	    msg_arr.push_back(message.substr(st_ind, ind-st_ind+1));
	  }
	  ind++;
	}
  } while (ind < message.size());

  return ret_val;
}

msg_ostream::msg_ostream(const char* msg_id, ostream &dest, msg_sev sev) :
	args_cnt(0),
	msg_class(msg_id, sev),
	destination_stream(dest) 
{
  if (parse_message() == 0)
    next_in_queue = 0;
  else
    next_in_queue = -1;

  if (get_original_severity() != normal_sev) {
    const char *sevp = sev_str[get_original_severity()];
    if (sevp[0] != '\0') {
      strm_out << sevp << ": ";
    }
  }
  strm_out << get_next_msg_from_arr();
}

msg_ostream::msg_ostream(const char* msg_id, const char *format_str, ostream &dest, msg_sev sev) :
	args_cnt(0),
	msg_class(msg_id, format_str, sev),
	destination_stream(dest) 
{
  if (parse_message() == 0)
    next_in_queue = 0;
  else
    next_in_queue = -1;

  if (get_original_severity() != normal_sev) {
    const char *sevp = sev_str[get_original_severity()];
    if (sevp[0] != '\0') {
      strm_out << sevp << ": ";
    }
  }
  strm_out << get_next_msg_from_arr();
}

msg_ostream::~msg_ostream()
{
}

//=========================================================================

static map<string, msg_traits> *message_map = 0;

void register_ui_callbacks(void (*popup_ptr)(msg_sev, char const *), void (*status_ptr)(msg_sev, char const *))
{
  popup_msg_callback = popup_ptr;
  status_msg_callback = status_ptr;
}

// Interprets a string that's encoded as a C
// string literal, i.e. enclosed in double quotes ""
// and using backslash (\) as an escape character.
static string interpret_string(const string &lit)
{
    if (lit.empty()) {
	return string();
    }
    int n_enclosing_quotes = 0;
    string::const_iterator i;
#ifdef ISO_CPP_HEADERS
    ostringstream val;
#else
    ostrstream val;
#endif
    char ch = 0;
    char first_ch = 0;
    i = lit.begin();
    for (i = lit.begin(); i != lit.end(); ++i) {
	ch = *i;
	if (i == lit.begin()) {
	    first_ch = ch;
	}
	if (ch == '\\') {
	    ++i;
	    ch = *i;
	    switch (ch) {
	    case '\'':
		val << '\'';
		break;
	    case '"':
		ch = '\\'; // ensure proper is_good check below
		val << '"';
		break;
	    case '?':
		val << '?';
		break;
	    case '\\':
		val << '\\';
		break;
	    case 'a':
		val << '\x7';  // ^G
		break;
	    case 'b':
		val << '\x8';  // ^H
		break;
	    case 'f':
		val << '\xC';  // ^L
		break;
	    case 'n':
#ifdef _WIN32
		val << '\xD';  // ^M
#endif
		val << '\xA';  // ^J
		break;
	    case 'r':
		val << '\xD';  // ^M
		break;
	    case 't':
		val << '\xC';  // ^L
		break;
	    case 'v':
		val << '\xB';  // ^K
		break;
	    case 'x':
		{
		    // Handle hex number.
		    unsigned long int_val = 0;
		    while (++i, ch = *i,
			      ('0' <= ch && ch <= '9')
			   || ('A' <= ch && ch <= 'F')
			   || ('a' <= ch && ch <= 'f')) {
			int_val = int_val * 16 + 
			  ('0' <= ch && ch <= '9' ? ch - '0' :
			   'A' <= ch && ch <= 'F' ? ch - 'A' + 10:
						    ch - 'a' + 10);
		    }
		    --i;
		    ch = '\\';
		    val << (char)int_val;
		}
		break;
	    default:
		{
		    // Handle octal number or other character.
		    int dig_count = 0;
		    int int_val = 0;
		    while ('0' <= ch && ch <= '7' && dig_count < 3) {
			int_val = int_val * 8 + (ch - '0');
			dig_count += 1;
			++i;
			ch = *i;
		    }
		    if (dig_count > 0) {
			// is octal
			val << (char)int_val;
			--i;
			ch = '\\';
		    }
		    else {
    	    	    	// the undefined case; pass along the escaped char
			val << ch;
		    }
		}
		break;
	    }
	}
	else if (ch == '"') {
	    n_enclosing_quotes += 1;
	}
	else {
    	    val << ch;
	}
    }
    bool is_good = (n_enclosing_quotes == 2 && ch == '"' && first_ch == '"');
#ifndef ISO_CPP_HEADERS
    val << ends;
#endif
    return val.str();
}

static bool initialize_msg_database()
{
  bool val = true;
  if (!message_map) {
    message_map = new map<string, msg_traits>;
    if (message_map) {
      char* psethomeEnvVar = getenv("PSETHOME");
      string file;
	  if(psethomeEnvVar == NULL)
		file = "";
	  else
		file = psethomeEnvVar;
      file.append("/lib/msg_database.dat");
      ifstream f(file.c_str(), ios::in);
      if (f.good()) {
	string line;
	while(f.good()) {
	  getline(f, line);
      	  if (!line.empty() && line[0] != '#') {
	    int tab1 = line.find('\t');
	    int tab2 = line.find('\t', tab1+1);
	    if (tab1 != string::npos && tab2 != string::npos && tab2 > tab1) {
	      string msgid    = line.substr(0, tab1);
	      string features = line.substr(tab1+1, tab2-tab1-1);
	      string format   = line.substr(tab2+1);

	      msg_traits t;

	      t.verbose = (   (features.find('v') != string::npos)
			   || (features.find('V') != string::npos));

	      if (   (features.find('p') != string::npos)
		  || (features.find('P') != string::npos)) {
		t.disposition = popup;
	      }
	      else if (   (features.find('s') != string::npos)
		       || (features.find('S') != string::npos)) {
		t.disposition = status;
	      }
	      else {
		t.disposition = standard;
	      }

	      t.format_str = interpret_string(format);

	      message_map->insert(pair<const string, msg_traits>(msgid, t));
	    }
	  }
	}
#ifndef _WIN32
        f.close();
#else
        //Leaving the closing of file up to the destructor, explicitly closing 
        //the file here with close() fails on Windows.
#endif
      } 
      else {
	//
	// consider this REAL carefully
	//
	msg("Cannot open file $1 for reading message database.", error_sev) << file << eoarg << eom;
	delete message_map;
	message_map = 0;
	val = false;
      }
    }
    else {
      val = false;
    }
  }

  if (val) {
    const char *vrb = getenv("DISCOVER_VERBOSE");
    if (!vrb || !strcmp(vrb, "no") || !strcmp(vrb, "0") || !strcmp(vrb, "NO")) {
      generate_verbose_output = false;
    }
  }

  return val;
}

bool search_msg_database(char const *msgid, msg_traits &t)
{
  static bool msg_database_initialized = false;
  bool val = false;

  if (!msg_database_initialized) {
    msg_database_initialized = true;
    initialize_msg_database();
  }

  if (message_map) {
    string m = msgid;
    map<string, msg_traits>::iterator p = message_map->find(m);
    if (p != message_map->end()) {
      val = true;
      t = p->second;
    }
  }
  return val;
}


// ===========================================================================

#ifdef STANDALONE	/* for testing purposes */

int main(unsigned int argc, char **argv)
{
  msg("In main function with $1 number of args \n") << argc << eoarg << eom;
  msg("$1 could not be opened in dir $2") << "a.dat" << eoarg << "c:/test" << eoarg << eom;
  msg("$1$2 could not be opened for reading") << "c:/test/" << eoarg << "b.dat" << eoarg << eom;
  msg("$1$2 could not be opened for reading") << "c:/test/" << eoarg << eom;
  msg("Could not open $1/$2 for reading") << "c:/test/" << eoarg << "b.dat" << eoarg<< eom;
  msg("$1$2$3$4") << "01" << eoarg << "02" << eoarg << "03" << eoarg << "04" <<eoarg <<eom;
  msg("FATAL ERROR! QUITING....") << eom;
  msg("Ignore \\$") << eom;
  msg("***") << eom;
  return 0;
}
#endif

