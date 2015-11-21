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

#include "service.h"
#include "mbuildmsg.h"
#include "mbmsgser.h"
using namespace MBDriver;
using namespace MiniXML;

#include "msg_comms.h"
#include "msg_format.h"

#include <stdio.h>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif /* _WIN32 */

#define ARG_KEY '$'
#define BACK_SLASH '\\'


static void initialize_xml_and_threads();
static const char *lp_service_name = NULL;
extern ostream_pool *stream_pool;

bool msg_viewer_is_running()
{
    return lp_service_name != NULL;
}


/*******************
 msg() functions
*******************/
ostream& msg(const char* msg_id)
{
  // don't use default arguments--they screw up the interface.  Forward instead:
  return msg(msg_id, unknown_sev);
}

extern void (*popup_msg_callback)(msg_sev, char const *);
extern void (*status_msg_callback)(msg_sev, char const *);
extern bool generate_verbose_output;

ostream& msg(const char* msg_id, msg_sev severity)
{
  //lookup message in database
  msg_traits t;
  bool found_in_database = search_msg_database(msg_id, t);

  //instantiate msg_socket or msg_ostream object here
  msg_class *mc;
  if (found_in_database) {
    if (msg_viewer_is_running())
      mc = new msg_socket(msg_id, t.format_str.c_str(), severity);
    else
      mc = new msg_ostream(msg_id, t.format_str.c_str(), cerr, severity);

    mc->set_disposition(t.disposition);
    mc->set_verbose_flag(t.verbose);
  }
  else {
    if (msg_viewer_is_running())
      mc = new msg_socket(msg_id, severity);
    else {
      mc = new msg_ostream(msg_id, cerr, severity);
    }
  }
  ostream* out = mc->received_msg();

  return *out;
}


/******************************************
Member functions for class msg_socket
*******************************************/

ostream& msg_socket::received_eoarg(ostream& out)
{
  //create a new ostringstream to store the next argument
  ostream *new_out = create_new_stream();
  stream_pool->remove_stream_from_pool(&out);
  stream_pool->add_stream_to_pool(new_out, this);

  return *new_out;
}


static int in_destruction = 0;
struct LP_connection {
    LP_connection() : messageSerializer(NULL), TCP_connection(NULL), non_sender(NULL) {}
    ~LP_connection() {
	in_destruction = 1;
	delete TCP_connection;  // closes the socket
	TCP_connection = NULL;
	delete non_sender;
	non_sender = NULL;
	delete messageSerializer;
	messageSerializer = NULL;
    }
    void init() {
	if (messageSerializer == NULL) {
	    messageSerializer = new MessageSerializer;
	    non_sender = new NonSender<Message>(*messageSerializer);
	}
    }
    MessageSerializer *messageSerializer;
    Connection<Message> *TCP_connection;
    NonSender<Message> *non_sender;
} connection_objs;

bool send_to_log_presentation(Message &msg) {
    bool sent_OK = false;
    if (lp_service_name == NULL) {
    }
    else if (!in_destruction) {
	initialize_xml_and_threads();
        if (connection_objs.TCP_connection == NULL) {
	    // Open the connection to log presentation, if it is not already open.
	    static ServicePort send_port(lp_service_name);
	    connection_objs.init();

	    if (!connect(send_port, *connection_objs.non_sender, NULL, &connection_objs.TCP_connection)) {
		cerr << "Could not send xml message, failed connecting to log presentation server: "
		     << lp_service_name << endl;
	    }
	}
	if (connection_objs.TCP_connection != NULL && connection_objs.TCP_connection->send(msg)) {
	    sent_OK = true;
	}
	else {
	    cerr << "Failure while sending to log presentation server: "
		 << lp_service_name << endl;
	}
    }
    else {
	cerr << "Failure while sending to log presentation server. Connection has been closed."
	     << endl;
    }
    return sent_OK;
}

void msg_socket::received_eom(ostream& out)
{
  if (!get_verbose_flag() || generate_verbose_output) {
    switch(get_disposition()) {
    case popup:
      if (popup_msg_callback)
	popup_msg_callback(get_original_severity(), get_format_string().c_str());
      break;
    case status:
      if (status_msg_callback)
	status_msg_callback(get_original_severity(), get_format_string().c_str());
      break;
    }
  }

  //received end of message, create xml object of the message and send
  //it on the socket to log presentation server.
  Message *xml_obj = create_xml_object();
  if (xml_obj) {
    if (!send_to_log_presentation(*xml_obj)) {
	cerr << "Failure while sending XML message." << endl;
    }
    delete xml_obj; 
 }

  return;
}

ostream* msg_socket::received_msg()
{
  //initialize stream_pool
  if (!stream_pool)
    stream_pool = new ostream_pool;

  ostream* new_out = create_new_stream();
  //add new stream to pool 
  stream_pool->add_stream_to_pool(new_out, this);

  return new_out;
}

ostream* msg_socket::create_new_stream()
{
  //create new ostringstream/ostrstream and add it to array of streams
#ifdef ISO_CPP_HEADERS
  ostream* out = new ostringstream();
#ifdef sun5
  // Hack around an ugly bug: failure if the first string written
  // to the stream is larger than 128 characters.
  out->write(" ", 1);
#endif
#else
  ostream* out = new ostrstream();
#endif
  arg_streams.push_back(out);
  return out;
}

msg_socket::msg_socket() : msg_class()
{
}

msg_socket::msg_socket(const char* msg_id, msg_sev severity)
  : msg_class(msg_id, severity)
{
}

msg_socket::msg_socket(const char* msg_id, const char *format_str, msg_sev severity)
  : msg_class(msg_id, format_str, severity)
{
}

msg_socket::~msg_socket() 
{
  ostream* os;
  for(unsigned int i = 0; i < arg_streams.size(); i++) {
    os = arg_streams[i];
	if (os) delete os;
  }
}

static void initialize_xml_and_threads()
{
  static bool initialized = false;
  if (initialized == false) {

#ifdef TRANSPORT_SERVER
    // Initialize thread system.
    SAThreads::initialize();
#endif
    initialized = true;
  }
}

void msg_socket::initialize_xml()
{
  initialize_xml_and_threads();
}

Message* msg_socket::create_xml_object()
{
  initialize_xml();

  Message *xml_obj = 0;
  int severity = get_original_severity();

  if (severity > (int)unknown_sev)
    severity = (int)unknown_sev;

  xml_obj = new Message(trivialToDOMString("msg"));
  if (xml_obj) {
    const string& msg_str = get_format_string();
    xml_obj->SetText(trivialToDOMString(msg_str));
    xml_obj->AddAttribute(trivialToDOMString("parent"), trivialToDOMString(AutoMessageGroup::current()));
    xml_obj->AddAttribute(trivialToDOMString("sev"), trivialToDOMString(sev_str[severity]));

    //add each argument of message as child of this node
    for(unsigned int i = 0; i < arg_streams.size(); i++) {
      // ostringstream *os = dynamic_cast<ostringstream *>(arg_streams[i]);
      // dynamic cast removed owing to warning from MS VC++:
      // dynamic cast used on polymorphic type 'class basic_stream<char,struct char_traits<char>>' with /GR-; unpredictable behavior may result
#ifdef ISO_CPP_HEADERS
      ostringstream *os = (ostringstream *)arg_streams[i];
#else
      ostrstream *os = (ostrstream *)arg_streams[i];
#endif
      if (os) {
	// Create a new Message object.
	Message *ch_obj = new Message(trivialToDOMString("arg"));
	if (ch_obj) {
#ifdef ISO_CPP_HEADERS
	  string arg_str = os->str();
#else
	  string arg_str;
	  if ( os && os->pcount()) {
	      (*os) << ends;
	      arg_str = os->str();
	  }
#endif
#ifndef sun5
	  ch_obj->SetText(trivialToDOMString(arg_str));
#else
	  // See above re: hack.
	  ch_obj->SetText(trivialToDOMString(arg_str.c_str()+1));
#endif

	  char num_str[10];
	  sprintf((char*)num_str, "%d", i+1);
	  ch_obj->AddAttribute(trivialToDOMString("num"), trivialToDOMString(num_str));
 
	  //make ch_obj child of xml_obj
	  xml_obj->AddChild(*ch_obj);
	  delete ch_obj;
	}
      }
    }
  }
  else {
    cerr << "Could not create xml object from message.\n";
  }

  return xml_obj;
}
// 
// ===========================================================================
// 

AutoMessageGroup_manager::AutoMessageGroup_manager() :
	servicePort(NULL),
	msgid_end(0),
	msgid_cur(0)
{
}

void
AutoMessageGroup_manager::pop()
{
	id_stack.pop();
}

void
AutoMessageGroup_manager::push(unsigned long id)
{
	id_stack.push(id);
}


// ===========================================================================
// static member function
// 
void
AutoMessageGroup_manager::init(const ServicePort &sp, const char *message_group_id)
{
    unsigned long parent_id = strtoul(message_group_id, NULL, 10);
    push(parent_id);

    if (servicePort) {
    	delete servicePort;
    }
    servicePort = new ServicePort(sp);
}

// ===========================================================================
// static member function
// 
const char *AutoMessageGroup_manager::current()
{
    static char str[25];
    if (! id_stack.empty() ) {
    	unsigned long l_tmp = id_stack.top();
	sprintf(str, "%lu", l_tmp);
	return str;
    }
    else {
	return NULL;
    }
}


static AutoMessageGroup_manager amgm;

// 
// ===========================================================================
// 

AutoMessageGroup::AutoMessageGroup(char const *name)
{
    amgm.report_new_group(name);
}

void
AutoMessageGroup_manager::report_new_group(char const *name)
{
    if (name == NULL) {
	name = "";
    }

    initialize_xml_and_threads();
    unsigned long parent_id = 0;
    if (! id_stack.empty() ) {
	parent_id = id_stack.top();
    }
    unsigned long id = parent_id;  // default to parent.

    Message *mg_query = new Message(trivialToDOMString("mgid"));
    if (msgid_cur >= msgid_end) {  // need ID from server
	if (mg_query != NULL && !in_destruction) {
	    mg_query->AddAttribute(trivialToDOMString("get"), &chNull);
	    Message *mg_response = NULL;
	    connection_objs.init();
	    QuerySender<Message> sndr(*mg_query, mg_response, *connection_objs.messageSerializer);
	    if (servicePort != NULL && connect(*servicePort, sndr)
				    && mg_response != NULL) {
		// OK
		DOMString tag;
		if (mg_response->GetTag(tag), tag.equals(trivialToDOMString("mgid"))) {
		    DOMString val;
		    if (mg_response->GetAttributeByName(trivialToDOMString("val"), val)) {
			char *v = trivialFromDOMString(val);
			unsigned int new_id = strtoul(v, NULL, 10);
			if (new_id > 0) {
			    msgid_cur = new_id;
			    msgid_end = msgid_cur + 1;
			}
			delete [] v;
		    }
		}
	    }
	    else {
		// The log presentation did not get a response.
	    }
	    delete mg_query;
	}
    }
    if (msgid_cur >= msgid_end) {
	// Query for new Id failed. Recover.
	push(id);
	return;
    }
    id = msgid_cur;
    ++msgid_cur;

    // Create xml object of the spawn event and send
    // it on the socket to log presentation server.
    Message *xml_obj = new Message(trivialToDOMString("spawn"));
    if (xml_obj) {
	char num_str[25];
	xml_obj->SetText(trivialToDOMString(name));
	sprintf((char *)num_str,"%lu", parent_id);
	xml_obj->AddAttribute(trivialToDOMString("parent"), trivialToDOMString(num_str));
	sprintf((char *)num_str,"%lu", id);
	xml_obj->AddAttribute(trivialToDOMString("child"), trivialToDOMString(num_str));

	if (send_to_log_presentation(*xml_obj)) {
	    // OK
	}
	else {
	    // The log presentation does not have the new group.  Recover.
	    id = parent_id;
	}
	delete xml_obj;
    }
    push(id);
    return;
}

AutoMessageGroup::~AutoMessageGroup()
{
	amgm.pop();
}

// static member function
char const *
AutoMessageGroup::current()
{
	return amgm.current();
}

// static member function
unsigned long AutoMessageGroup::id_str_to_num(const char *id_str)
{
    return strtoul(id_str, NULL, 10);
}

AutoMessageGroup::AutoMessageGroup(unsigned long id)
{
    amgm.push(id);
}

// Note: default and copy constructor of AutoMessageGroup are not defined,
// because uncontrolled construction of these objects would lead to
// uncontrolled destruction, which would involve extraneous calls to
// amgm.pop().  These constructors are implementable if you add some
// data to the class, but that's tedious and we can get by without them
// for now.

// ===========================================================================
void init_lp_service(const char *log_presentation_service_name,
		     const char *message_group)
{
    lp_service_name = log_presentation_service_name;
    amgm.init(ServicePort(log_presentation_service_name), message_group);
}


// ===========================================================================
void report_progress(const char *progress_msg)
{
    if (msg_viewer_is_running() && progress_msg != NULL) {
        initialize_xml_and_threads();
        Message *prog_msg = new Message(trivialToDOMString("progress"));
        if (prog_msg) {
            prog_msg->AddAttribute(trivialToDOMString("done"), trivialToDOMString(progress_msg));

            if ( !send_to_log_presentation(*prog_msg) ) {
                cerr << "Could not send progress message." << endl;
            }

            delete prog_msg;
        }
    }
}

// ===========================================================================
// Specify that a task consists of a number of equal steps.
// task: name of task that consists of steps
// step: name of one step
// third arg: number of steps expected to complete the task
void define_progress_steps(const char *task, const char *step,
			   unsigned long n_steps)
{
    if (msg_viewer_is_running() && task != NULL && step != NULL) {
        initialize_xml_and_threads();
        Message *steps_msg = new Message(trivialToDOMString("steps"));
        if (steps_msg) {
            steps_msg->AddAttribute(trivialToDOMString("name"), trivialToDOMString(step));
            steps_msg->AddAttribute(trivialToDOMString("for"), trivialToDOMString(task));
	    char steps_buf[25];
	    sprintf(steps_buf, "%lu", n_steps);
            steps_msg->AddAttribute(trivialToDOMString("count"), trivialToDOMString(steps_buf));

            if ( !send_to_log_presentation(*steps_msg) ) {
                cerr << "Could not send steps message." << endl;
            }

            delete steps_msg;
        }
    }
}

// ===========================================================================
//The following procedures is used put an existing message group id on to 
//the stack so that messages can be sent to it.
void activate_message_group(const char* grpid_str)
{
  unsigned long message_group_id = strtoul(grpid_str, NULL, 10);
  if (message_group_id != 0) {
      amgm.push(message_group_id);
  }
  return;
}

//The following procedure is used (after activate_message_group) to 
//remove the top most message group id from the stack.
void deactivate_message_group(void)
{
  amgm.pop();
  return;
}
//=============================================================================
