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
#include "service.h"
#include "mbuildmsg.h"
#include "msgreceiver.h"
#include "msgsender.h"
#include "logpresent.h"
#include "lpinterface.h"
#include <math.h>
#include <vector>
#include <list>
#include "mbmsgser.h"
#include "filtersev.h"
#include "minixml.h"
#include "minidom.h"
#include "threads.h"

#ifdef ISO_CPP_HEADERS
#include <sstream>
#include <iostream>
#else
#include <strstream.h>
#include <iostream.h>
#endif

namespace std {};
using namespace std;
using namespace nTransport;
using namespace MBDriver;
using namespace MiniXML;
using namespace SAThreads;

static string lpservice_name;
static string passservice_name;

static bool done_sent = false;
static bool mb_done = false;
static bool mb_complete = false;

static void reset_doneness()
{
    done_sent = false;
    mb_done = false;
    mb_complete = false;
}

static bool trace_sends = false;

static void dbgprt(const char *str) {
    cerr << str << endl << flush;
}

static const MiniXML::XMLCh argXMLStr[] = 
{
	MiniXML::chLatin_a, MiniXML::chLatin_r, MiniXML::chLatin_g, MiniXML::chNull
};
static const MiniXML::XMLCh brXMLStr[] = 
{
	MiniXML::chLatin_b, MiniXML::chLatin_r, MiniXML::chNull
};
static const XMLCh breakdownXMLStr[] =
{
    chLatin_b, chLatin_r, chLatin_e, chLatin_a, chLatin_k, chLatin_d, chLatin_o, chLatin_w, chLatin_n, chNull
};
static const MiniXML::XMLCh catastropheXMLStr[] = 
{
	MiniXML::chLatin_c, MiniXML::chLatin_a, MiniXML::chLatin_t, MiniXML::chLatin_a, MiniXML::chLatin_s, MiniXML::chLatin_t, MiniXML::chLatin_r, MiniXML::chLatin_o, MiniXML::chLatin_p, MiniXML::chLatin_h, MiniXML::chLatin_e, MiniXML::chNull
};
static const MiniXML::XMLCh childXMLStr[] = 
{
	MiniXML::chLatin_c, MiniXML::chLatin_h, MiniXML::chLatin_i, MiniXML::chLatin_l, MiniXML::chLatin_d, MiniXML::chNull
};
static const XMLCh countXMLStr[] =
{
	chLatin_c, chLatin_o, chLatin_u, chLatin_n, chLatin_t, chNull
};
static const MiniXML::XMLCh doneXMLStr[] = 
{
	MiniXML::chLatin_d, MiniXML::chLatin_o, MiniXML::chLatin_n, MiniXML::chLatin_e, MiniXML::chNull
};
static const XMLCh forXMLStr[] =
{
	chLatin_f, chLatin_o, chLatin_r, chNull
};
static const MiniXML::XMLCh helpXMLStr[] = 
{
	MiniXML::chLatin_h, MiniXML::chLatin_e, MiniXML::chLatin_l, MiniXML::chLatin_p, MiniXML::chNull
};
static const MiniXML::XMLCh msgXMLStr[] = 
{
	MiniXML::chLatin_m, MiniXML::chLatin_s, MiniXML::chLatin_g, MiniXML::chNull
};
static const MiniXML::XMLCh messageXMLStr[] = 
{
	MiniXML::chLatin_m, MiniXML::chLatin_e, MiniXML::chLatin_s, MiniXML::chLatin_s, MiniXML::chLatin_a, MiniXML::chLatin_g, MiniXML::chLatin_e, MiniXML::chNull
};
static const XMLCh nameXMLStr[] =
{
    chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull
};
static const MiniXML::XMLCh nodeXMLStr[] = 
{
	MiniXML::chLatin_n, MiniXML::chLatin_o, MiniXML::chLatin_d, MiniXML::chLatin_e, MiniXML::chNull
};
static const MiniXML::XMLCh normalXMLStr[] = 
{
	MiniXML::chLatin_n, MiniXML::chLatin_o, MiniXML::chLatin_r, MiniXML::chLatin_m, MiniXML::chLatin_a, MiniXML::chLatin_l, MiniXML::chNull
};
static const MiniXML::XMLCh numXMLStr[] = 
{
	MiniXML::chLatin_n, MiniXML::chLatin_u, MiniXML::chLatin_m, MiniXML::chNull
};
static const MiniXML::XMLCh parentXMLStr[] = 
{
	MiniXML::chLatin_p, MiniXML::chLatin_a, MiniXML::chLatin_r, MiniXML::chLatin_e, MiniXML::chLatin_n, MiniXML::chLatin_t, MiniXML::chNull
};
static const MiniXML::XMLCh progressXMLStr[] = 
{
	MiniXML::chLatin_p, MiniXML::chLatin_r, MiniXML::chLatin_o, MiniXML::chLatin_g, MiniXML::chLatin_r, MiniXML::chLatin_e, MiniXML::chLatin_s, MiniXML::chLatin_s, MiniXML::chNull
};
static const MiniXML::XMLCh reviseXMLStr[] = 
{
	MiniXML::chLatin_r, MiniXML::chLatin_e, MiniXML::chLatin_v, MiniXML::chLatin_i, MiniXML::chLatin_s, MiniXML::chLatin_e, MiniXML::chNull
};
static const MiniXML::XMLCh sevXMLStr[] = 
{
	MiniXML::chLatin_s, MiniXML::chLatin_e, MiniXML::chLatin_v, MiniXML::chNull
};
static const MiniXML::XMLCh severityXMLStr[] = 
{
	MiniXML::chLatin_s, MiniXML::chLatin_e, MiniXML::chLatin_v, MiniXML::chLatin_e, MiniXML::chLatin_r, MiniXML::chLatin_i, MiniXML::chLatin_t, MiniXML::chLatin_y, MiniXML::chNull
};
static const XMLCh sizeXMLStr[] =
{
	chLatin_s, chLatin_i, chLatin_z, chLatin_e, chNull
};
static const MiniXML::XMLCh spawnXMLStr[] = 
{
	MiniXML::chLatin_s, MiniXML::chLatin_p, MiniXML::chLatin_a, MiniXML::chLatin_w, MiniXML::chLatin_n, MiniXML::chNull
};
static const XMLCh stepsXMLStr[] =
{
    chLatin_s, chLatin_t, chLatin_e, chLatin_p, chLatin_s, chNull
};
static const XMLCh taskXMLStr[] =
{
    chLatin_t, chLatin_a, chLatin_s, chLatin_k, chNull
};
static const MiniXML::XMLCh zeroXMLStr[] = 
{
    MiniXML::chDigit_0, MiniXML::chNull
};

bool is_null_service(const string &svc_name) {
    return svc_name == "" || svc_name == "0"
		|| svc_name == "null" || svc_name == "Null"
		|| svc_name == "none" || svc_name == "None";
}

//Global list to hold all messages. Access to the list is synchronized
//so that only one thread can either add a message or remove a message.
MessageList messages_list;

// ===========================================================================
void MsgProcessor::run()
{
    if (is_null_service(passservice_name)) {
	processAllMessages(NULL);
    }
    else {
	MessageSender sender(*this);
	string *diagnostic_p;
	cerr << "About to connect to mbdriver passthrough server." << endl << flush;
	if ( !connect(passservice_name.c_str(), sender, &diagnostic_p) ) {
	    if (diagnostic_p == NULL) {
	      cerr << "Could not connect to mbdriver passthrough server: "
		   << passservice_name.c_str() << endl;
	    }
	    else {
	      cerr << "Could not connect to mbdriver passthrough server: "
		   << passservice_name.c_str()
		   << ", because: " << diagnostic_p->c_str() << endl;
	      delete diagnostic_p;
	    }
	}
	else {
	    cerr << "Finished with connection to mbdriver passthrough server." << endl << flush;
	}
    }
    delete this;
}

//This function gets the first entry from the global list and processes it.
void MsgProcessor::processAllMessages(Connection<Message> *conn)
{
  while (true) {
    Message *msg_obj = NULL;
    bool need_to_send_done = mb_done && !done_sent;
    {
	Exclusion synchronized(messages_list, "getting a message from list");
	while ( messages_list.size() <= 0 && !need_to_send_done ) {
	  messages_list.wait();
	}
	msg_obj = messages_list.popFirstMessage();
    }
    if (!msg_obj && need_to_send_done) {
	msg_obj = new Message(DOMString(doneXMLStr));
	done_sent = true;
    }
    if (msg_obj || need_to_send_done) { 
      vector<Message*> send_list;
      if (processOneMessage(msg_obj, send_list) || retrieveDoneMessage(send_list)) {
	for (int i = 0; i < send_list.size(); i++) {
	  Message *send_object = send_list[i];
	  // Send processed message to pass through server.
	  if (send_object && trace_sends) {
	      MessageSerializer s;
	      const char *data;
	      size_t length;
	      s.serialize(*send_object, data, length);
#ifdef ISO_CPP_HEADERS
	      stringstream strs;
#else
	      strstream strs;
#endif
	      strs << "sending: ";
	      for (int i = 4; i < length; i += 1) {
		  strs << data[i];
	      }
#ifndef ISO_CPP_HEADERS
	      strs << ends;
#endif
	      string strs_str = strs.str();
	      dbgprt(strs_str.c_str());
	  }
	  if (send_object && conn && !conn->send(*send_object)) {
	    if (conn->isError()) {
	      cerr << "Could not send message due to error on socket.\n";
	    }
	    else {
	      cerr << "Could not send message on socket.\n";
	    }
	    if (trace_sends) {
		dbgprt("sending failed");
	    }
	    // Don't just return, because the send_list should continue to
	    // be harvested.
	  }
	  // Delete after it has been sent.
	  if (send_object) delete send_object;
	}
      }
      delete msg_obj;
    }
  }
}

// ===========================================================================
// Sets the text of the given message, translating newline characters into
// <br/> elements.
// 
static void set_text(Message &send_obj, const string &text)
{
    const char *p = text.c_str();
    size_t length = strlen(p);
    char *buf = new char[length + 1];
    strcpy(buf, p);

    p = buf;
    for (size_t i = 0; i <= length; i += 1) {
	if (buf[i] == '\n') {
	    buf[i] = '\0';
	    DOMString domstr;
	    trivialToDOMString(domstr, p);
	    send_obj.AddText(domstr);
	    DOMString brDOMStr(brXMLStr);
	    Message br = brDOMStr;
	    send_obj.AddChild(br);
	    p = buf + i + 1;
	}
	else if (buf[i] == '\0') {
	    DOMString domstr;
	    trivialToDOMString(domstr, p);
	    send_obj.AddText(domstr);
	}
    }
    delete [] buf;
}

static void send_progress_message(unsigned short percent_done,
				  vector<Message*>& send_list)
{
    Message *send_obj = new Message(DOMString(progressXMLStr));
    if (send_obj) {
	char buf[25];
	sprintf(buf, "%d", percent_done);
	DOMString dom_buf;
	trivialToDOMString(dom_buf, buf);
	send_obj->AddAttribute(doneXMLStr, dom_buf);

	send_list.push_back(send_obj);
    }
}

bool is_prefix_of(const DOMString &pre, const DOMString &text) {
    int lpre = pre.length();
    if (lpre <= text.length()) {
	const XMLCh *prep = pre.rawBuffer();
	const XMLCh *textp = text.rawBuffer();
	for (int i = 0; i < lpre; i += 1) {
	    if (prep[i] != textp[i]) {
		return false;
	    }
	}
	return true;
    }
    else {
	return false;
    }
}

bool MsgProcessor::processOneMessage(Message *obj, vector<Message*>& send_list)
{
  bool ret_flag = true;
  DOMString obj_tag;
  if (obj && (obj->GetTag(obj_tag), obj_tag.equals(msgXMLStr))) {
      DOMString text;
      DOMString doneText;
      trivialToDOMString(doneText, COMMAND_DONE_TEXT);
      if (obj->GetText(text), is_prefix_of(doneText, text)) {
	  mb_done = true;
	  ret_flag = false;
	  const XMLCh *extrap = text.rawBuffer() + doneText.length();
	  int lextra = text.length() - doneText.length();
	  delete doneWhat;
	  doneWhat = lextra > 0 ? new DOMString(extrap, lextra) : NULL;
	  doneMessage = new Message(DOMString(doneXMLStr));
      }
      else {
	Message *send_obj = new Message(DOMString(messageXMLStr));
	if (send_obj) {
	  char buf[25];
	  sprintf(buf, "%lu", idPool.get());
	  DOMString dom_buf;
	  trivialToDOMString(dom_buf, buf);
	  send_obj->AddAttribute(nodeXMLStr, dom_buf);
	  send_obj->AddAttribute(helpXMLStr, zeroXMLStr);

	  // parent attribute
	  DOMString parent;
	  obj->GetAttributeByName(parentXMLStr, parent);
	  if (parent.length() > 0) {
  	    send_obj->AddAttribute(parentXMLStr, parent);
	  }
	  else {
  	    send_obj->AddAttribute(parentXMLStr, zeroXMLStr);
	  }
      
	  //add severity
	  DOMString severity;
	  obj->GetAttributeByName(sevXMLStr, severity);
	  send_obj->AddAttribute(severityXMLStr, severity);
  
	  string flat_msg = substitute_arguments(obj);
	  set_text(*send_obj, flat_msg);
	  //add send_obj to send_list
	  send_list.push_back(send_obj);

	  DOMString message_id_str;
	  obj->GetText(message_id_str);

	  //if severity is not "normal" propagate severity up 
	  if (!severity.equals(normalXMLStr)) {
	    propagate_severity_up(parent, severity, message_id_str, send_list);
	  }
	} else
	  ret_flag = false;
      }
  }
  else if (obj && (obj->GetTag(obj_tag), obj_tag.equals(spawnXMLStr))) {
    Message *send_obj = new Message(DOMString(messageXMLStr));
    if (send_obj) {
      send_obj->AddAttribute(helpXMLStr, zeroXMLStr);
      send_obj->AddAttribute(severityXMLStr, normalXMLStr);

      // child attribute
      DOMString child;
      obj->GetAttributeByName(childXMLStr, child);
      send_obj->AddAttribute(nodeXMLStr, child);

      // parent attribute
      DOMString parent;
      obj->GetAttributeByName(parentXMLStr, parent);
      if (parent.length() > 0) {
	send_obj->AddAttribute(parentXMLStr, parent);
      }
      else {
	send_obj->AddAttribute(parentXMLStr, zeroXMLStr);
      }
    
      // Convert the message group name to a message text.
      DOMString gpnm;
      obj->GetText(gpnm);
      char *group_name = trivialFromDOMString(gpnm);
#ifdef ISO_CPP_HEADERS
      stringstream msg_text;
#else
      strstream msg_text;
#endif
      msg_text << "Processing " << group_name << ".";
#ifndef ISO_CPP_HEADERS
      msg_text << ends;
#endif
      string msg_text_str = msg_text.str();
      DOMString dom_msg_text;
      trivialToDOMString(dom_msg_text, msg_text_str.c_str());
      send_obj->SetText(dom_msg_text);
      delete group_name;

      send_list.push_back(send_obj);
      //add the new message group id to the tree of message group ids
      mgid_tree_root.add_new_mgid(child, gpnm, parent);
    } else
      ret_flag = false;
  }
  else if (obj && (obj->GetTag(obj_tag), obj_tag.equals(taskXMLStr))) {
      DOMString name_str;
      obj->GetAttributeByName(nameXMLStr, name_str);
      delete main_task;
      main_task = new TaskProgress(name_str, (void *)this, progress_callback);
      percent_done = 0;
      reset_doneness();
      send_progress_message(0, send_list); // Mbdriver likes to know we're here.
  }
  else if (main_task != NULL && obj != NULL) {
      obj->GetTag(obj_tag);
      if (obj_tag.equals(breakdownXMLStr)||obj_tag.equals(stepsXMLStr)) {
	  main_task->process_breakdown_message(*obj);
	  ret_flag = false;
      }
      else if (obj_tag.equals(progressXMLStr)) {
	  DOMString done_name;
	  obj->GetAttributeByName(doneXMLStr, done_name);
	  unsigned short prev_percent_done = percent_done;
	  main_task->process_progress_message(*obj, done_name);
	  if (percent_done != prev_percent_done) {
	      send_progress_message(percent_done, send_list);
	  }
	  else {
	      ret_flag = false;
	  }
      }
  }
  else if (obj && (obj->GetTag(obj_tag), obj_tag.equals(progressXMLStr))) {
    //handle progress message
    DOMString prg_str;
    obj->GetAttributeByName(doneXMLStr, prg_str);
    unsigned short percentdone;
    if (progressProcessor.get_percentage(prg_str, percentdone, this)) {
      Message *send_obj = new Message(DOMString(progressXMLStr));
      if (send_obj) {
	char buf[25];
	sprintf(buf, "%d", percentdone);
	DOMString dom_buf;
	trivialToDOMString(dom_buf, buf);
	send_obj->AddAttribute(doneXMLStr, dom_buf);

	send_list.push_back(send_obj);
      }
    } else
      ret_flag = false;
  }
  else
    ret_flag = false;

  return ret_flag;
}

static DOMString capitalize(const DOMString &s)
{
    DOMString result;
    unsigned long len = s.length();
    if (len > 0) {
        result += (XMLCh)toupper(s.charAt(0));
	result += DOMString(s.rawBuffer() + 1, len - 1);
    }
    return result;
}

bool MsgProcessor::retrieveDoneMessage(vector<Message*>& send_list)
{
    bool ret_flag = false;
    if (doneMessage) {
	// add doneMessage to send_list
	if (!mb_complete) {
	    // Ensure GUI is informed of error, even if everything
	    // appeared OK until the end (i.e. in case model build 
	    // suddenly dies).
	    Message *send_obj = new Message(DOMString(messageXMLStr));
	    if (send_obj) {
	      char buf[25];
	      sprintf(buf, "%lu", idPool.get());
	      DOMString dom_buf;
	      trivialToDOMString(dom_buf, buf);
	      send_obj->AddAttribute(nodeXMLStr, dom_buf);
	      send_obj->AddAttribute(helpXMLStr, zeroXMLStr);
	      send_obj->AddAttribute(parentXMLStr, zeroXMLStr);
	      DOMString error_severity(catastropheXMLStr);
	      send_obj->AddAttribute(severityXMLStr, error_severity);
  
	      DOMString domstr;
	      if (main_task != NULL) {
		  domstr = capitalize(main_task->get_name());
		  domstr += trivialToDOMString(" is incomplete.");
	      }
	      else if (doneWhat != NULL) {
	          trivialToDOMString(domstr, "Execution of ");
		  domstr += (*doneWhat);
		  domstr += trivialToDOMString(" is incomplete.");
		  domstr += trivialToDOMString(" It did not even identify itself.");
	      }
	      else if (doneWhat != NULL) {
	          trivialToDOMString(domstr, "Model administration command failed without identifying itself.");
	      }
	      send_obj->SetText(domstr);
	      //add send_obj to send_list
	      send_list.push_back(send_obj);

	      //if severity is not "normal" propagate severity up 
	      DOMString root_gid(zeroXMLStr);
	      propagate_severity_up(root_gid, error_severity, domstr, send_list);
	    }
	}

	send_list.push_back(doneMessage);
	ret_flag = true;
	done_sent = true;
	doneMessage = NULL;
    }
    return ret_flag;
}

// ===========================================================================
// Substitute for the control characters in the given string,
// with suitable values for the XML text that is forwarded to the GUI.
//
static void substitute_for_control_chars(string &str,
					 bool pass_newlines,
					 bool trim_newlines)
{
#ifdef ISO_CPP_HEADERS
    ostringstream result;
#else
    ostrstream result;
#endif
    const char *cp = str.c_str();
    char ch;
    int deferred_newline_count = 0;
    bool at_beginning = true;
    while ((ch = *cp) != '\0') {
	if (0 <= ch && ch < '\x20') {
	    if (ch == '\x0A') {
		// new line
		if (trim_newlines) {
		    deferred_newline_count += 1;
		}
		else {
		    result << (pass_newlines ? "\n" : " ");
		}
	    }
	    else if (ch == '\x0D') {
		// carriage return
		if (cp[1] == '\x0A') {
		    // belongs with next newline; don't pass it along
		}
		else if (trim_newlines) {
		    deferred_newline_count += 1;
		}
		else {
		    result << (pass_newlines ? "\n" : " ");
		}
	    }
	    else if (ch == '\t') {
		// tab
		result << " ";
	    }
	    else {
		// Unexpected control charcter; don't pass it along.
	    }
	}
	else {
	    // normal character; pass it along
	    // First, emit deferred newlines if appropriate.
	    for (; deferred_newline_count > 0; deferred_newline_count -= 1) {
		if (!at_beginning) {
		    result << (pass_newlines ? "\n" : " ");
		}
	    }
	    result << ch;
	    at_beginning = false;
	}
	cp += 1;
    }
#ifndef ISO_CPP_HEADERS
    result << ends;
#endif
    str = result.str();
}

// ===========================================================================
// If a message is designed to have one or more multline arguments, it needs
// to be detected.  More messages may be detected here in the future.
//
static vector<bool> get_multiline_args(const char *msg_id) {
    vector<bool> result;
    if (strcmp(msg_id, "aset_CCcc: $1") == 0) {
	result.resize(2);
	result[1] = true;
    }
    return result;
}

// ===========================================================================
// Substitutes arguments for the message, returning the message as a flat
// string.  Also takes care of control characters that may occur in the
// text or arguments, except that newlines are preserved so that they can
// be turned into <br/> elements later.
//
string MsgProcessor::substitute_arguments(Message *obj)
{
  string flat_msg = "";
  if (obj) {
    DOMString dstr;
    obj->GetText(dstr);
    char *cstr = trivialFromDOMString(dstr);
    vector<bool> multiline_args = get_multiline_args(cstr);
    string msg_id = cstr;
    substitute_for_control_chars(msg_id, true, true);
 
    // Organize by arg num the argument objects in the message.
    int ch_enum;
    bool enum_flag = obj->EnumerateChildren(ch_enum);
    vector<Message *> args;
    for (;;) {
	Message *child;
	if (enum_flag && obj->GetNextChild(ch_enum, &child)) {
	    DOMString child_tag;
	    child->GetTag(child_tag);
	    if (child_tag.equals(argXMLStr)) {
	        DOMString arg_num_str;
		unsigned long arg_num = 0;
		if (child->GetAttributeByName(numXMLStr, arg_num_str)) {
		    char *cstr = trivialFromDOMString(arg_num_str);
		    arg_num = strtoul(cstr, 0, 10);
		    delete [] cstr;
		}
		if (args.size() <= arg_num) {
		    args.resize(arg_num + 1, NULL);
		}
		args[arg_num] = child;
	    }
	}
	else {
	    break;
	}
    }

    // Scan the 'message id', substituting the argument values.
    int found = -1;
    int st_ind = 0, len = msg_id.size();
    while ( st_ind < len ) {
      found = msg_id.find("$", st_ind);
      if (found >= 0) {
	// Copy up to the $.
	flat_msg += msg_id.substr(st_ind, found - st_ind);
	st_ind = found;
	// Obtain arg num from the digits following the $.
#ifdef ISO_CPP_HEADERS
	ostringstream digs;
#else
	ostrstream digs;
#endif
	while (++st_ind < len && isdigit(msg_id[st_ind])) {
	    digs << msg_id[st_ind]; 
	}
#ifndef ISO_CPP_HEADERS
	digs << ends;
#endif
	string digs_str = digs.str();
	unsigned long arg_num = strtoul(digs_str.c_str(), NULL, 10);
	// Select the child and copy its text.
	Message *child = arg_num < args.size() ? args[arg_num] : NULL;
	if (child != NULL) {
	  DOMString darg;
          if (child) {
	    child->GetText(darg);
            char *s = trivialFromDOMString(darg);
	    string argstr(s);
	    bool multi = arg_num < multiline_args.size()
		        && multiline_args[arg_num];
	    substitute_for_control_chars(argstr, multi, multi);
	    flat_msg += argstr;
	    delete s;
	  }
	}
	else {
	    // The argument value is not present. Copy the parameter number.
	    flat_msg += msg_id.substr(found, st_ind - found);
	}
      } else { 
	// Copy the remainder of the 'message id'.
	flat_msg += msg_id.substr(st_ind, len - st_ind);
	break;
      }
    }
    delete cstr;
  }

  return flat_msg;
}

void MsgProcessor::propagate_severity_up(DOMString& gid_str,
					 DOMString& sev_str,
					 DOMString &node_dom_str,
					 vector<Message*>& send_list)
{
  unsigned long gid = 0;
  if (gid_str.length() > 0) {
    char *cstr = trivialFromDOMString(gid_str);
    if (cstr)
      gid = atol(cstr);
    delete cstr;
  }
  char *sstr = trivialFromDOMString(sev_str);
  if (sstr) {
    lp_msg_sev new_sev = severity_from_string(sstr);
    string child_str = trivialStringFromDOMString(node_dom_str);
    bool child_is_message_group = false;
    MsgGroupIdNode *parent_node = mgid_tree_root.find_mgid_node(gid);
    //while node has parent change its severity if it is less than the new 
    //severity. Also send a "revise" message to GUI to change severity.
    while (parent_node) {
      // Refine the severity which gets bubbled up.
      new_sev = filter_severity(parent_node->getName(),
			        child_str, 
			        new_sev,
				child_is_message_group);
      if (parent_node->get_severity() < new_sev && new_sev < explicit_unknown_lp_sev) {
	parent_node->put_severity(new_sev);

	Message *send_obj = new Message(DOMString(reviseXMLStr));
	if (send_obj) {
	  unsigned long parent_mgid = parent_node->get_mgid();
	  char buf[25];
	  sprintf(buf, "%lu", parent_mgid);
	  DOMString dom_buf;
	  trivialToDOMString(dom_buf, buf);
	  send_obj->AddAttribute(nodeXMLStr, dom_buf);
	  DOMString dom_new_sev;
	  trivialToDOMString(dom_new_sev, string_from_severity(new_sev));
	  send_obj->AddAttribute(severityXMLStr, dom_new_sev);

	  send_list.push_back(send_obj);
	}
      }
      child_is_message_group = true;
      child_str = parent_node->getName();
      parent_node = parent_node->get_parent();
    }
    delete sstr;
  }
}

void MsgProcessor::reset()
{
    mgid_tree_root.reset();
    delete main_task;
    main_task = NULL;
}

// static member function
void MsgProcessor::progress_callback(void *obj, double val) {
    ((MsgProcessor *)obj)->note_progress(val);
}

void MsgProcessor::note_progress(double val) {
    if (val == 1.0) {
	mb_complete = true;
    }
    unsigned short new_percent_done = (unsigned short)(floor(100*val));
    if (percent_done < new_percent_done && new_percent_done <= 100) {
	percent_done = new_percent_done;
    }
}

//===========================================================================
// MessageList member functions

// Synchronized function to add a new message to list
void MessageList::addMessage(Message *obj)
{
    Exclusion synchronized(*this, "adding a message to list");
    if (obj) {
	messages.push_back(obj);
	notify();
    }
}

Message* MessageList::popFirstMessage()
{
  Message* obj = 0;
  list<Message*>::iterator it = messages.begin();
  if (it != messages.end()) {
    obj = *it;
    //remove the entry from the list
    messages.erase(it);
  }
  return obj;
}

// ===========================================================================
//
unsigned long IdPool::get()
{
  Exclusion synchronized(*this, "getting an id");
  unsigned long val = available;
  available += 1;
  return val;
}

//===========================================================================
TrackProgress::TrackProgress(unsigned short p) :
    percent(p),
    current_percent(0.0),
    current_file_count(0.0),
    files_before_incr(0.0),
    increments(0.0)
{
}

void TrackProgress::initialize_counts(int count) 
{
    if ( count > 0 && percent > 0) {
	if (percent >= count) {
	    files_before_incr = 1.0;
	    increments = (float)percent/count;
	} else {
	    files_before_incr = (float)count/percent;
	    increments = 1.0;
	}
	current_percent = 0.0;
	current_file_count = 0.0;
    }
    else
	increments = 0.0;
}

float TrackProgress::incr_file_count()
{
    float incr = 0.0;
    current_file_count += 1.0;
    if (increments > 0.0 && current_file_count >= files_before_incr) {
	incr = increments;
	current_percent += increments;
	current_file_count = current_file_count - files_before_incr;
    }
    return incr;
}

float TrackProgress::done()
{
    return percent - current_percent;
}

// ==========================================================================
// ProgressMsgProcessor member functions

ProgressMsgProcessor::ProgressMsgProcessor()
{ 
  percent_done = 0.0;
  initialization = 0;
  flags_translation = 0;
  parse_pass_track = 0;
  update_pass_track = 0;
  finalize_pass = 0;
  qa_calliper_track = 0;
  reset(false); // Just so a failure to receive start message won't cause a crash.
}

ProgressMsgProcessor::~ProgressMsgProcessor()
{
    cleanup();
}

void ProgressMsgProcessor::reset(bool qa)
{
  cleanup();
  percent_done = 0.0;
  if (qa) {
    initialization = 2;
    flags_translation = 3;
    parse_pass_track = new TrackProgress(30);
    update_pass_track = new TrackProgress(10);
    finalize_pass = 2;
    qa_calliper_track = new TrackProgress(53);
  }
  else {
    initialization = 5;
    flags_translation = 5;
    parse_pass_track = new TrackProgress(65);
    update_pass_track = new TrackProgress(20);
    finalize_pass = 5;
    qa_calliper_track = 0;
  }
}

void ProgressMsgProcessor::cleanup()
{
    if (parse_pass_track) {
	delete parse_pass_track;
	parse_pass_track = 0;
    }
    if (update_pass_track) {
	delete update_pass_track;
	update_pass_track = 0;
    }
    if (qa_calliper_track) {
	delete qa_calliper_track;
	qa_calliper_track = 0;
    }
}

bool ProgressMsgProcessor::get_percentage(DOMString& str, unsigned short& done, MsgProcessor* msg_processor)
{
  bool has_changed = true;
  char *c_str = trivialFromDOMString(str);
  if (c_str) {
    if (strcmp(c_str, "start modelbuild") == 0           // from dspparser_driver.pl
	|| strcmp(c_str, "invoke model build") == 0     // from mbdriver.exe
	|| strcmp(c_str, "start modelbuildAndQA") == 0) {  // from dspparser_driver.pl 
      
      bool qa = (strcmp(c_str, "start modelbuildAndQA") == 0);
      reset(qa);
      msg_processor->reset();
      if (strcmp(c_str, "invoke model build") == 0) {   // from mbdriver.exe
        reset_doneness();
      }
    }
    else if (strcmp(c_str, "parse_file") == 0) {
	float p = parse_pass_track->incr_file_count();
	if (p > 0.0)
	    percent_done += p;
	else
	    has_changed = false;
    } 
    else if (strcmp(c_str, "update_file") == 0) {
	float p = update_pass_track->incr_file_count();
	if (p > 0.0)
	    percent_done += p;
	else
	    has_changed = false;
    }
    else if (strcmp(c_str, "qa_file") == 0) {
	float p = qa_calliper_track->incr_file_count();
	if (p > 0.0)
	    percent_done += p;
	else
	    has_changed = false;
    }
    else if (strcmp(c_str, "initialization") == 0) {
	percent_done += initialization;
    }
    else if (strcmp(c_str, "flags translation") == 0) {
	percent_done += flags_translation;
    }
    else if (strcmp(c_str, "parse pass") == 0) {
	float adjust = parse_pass_track->done();
	if (adjust > 0.0)
	    percent_done += adjust;
	else
	    has_changed = false;
    }
    else if (strncmp(c_str, "parse_file_count:", 17) == 0) {
	initialize_parse_count(c_str);
    }
    else if (strcmp(c_str, "update pass") == 0) {
	float adjust = update_pass_track->done();
	if (adjust > 0.0)
	    percent_done += adjust;
	else
	    has_changed = false;
    }
    else if (strncmp(c_str, "update_file_count:", 18) == 0) {
	initialize_update_count(c_str);
    }
    else if (strcmp(c_str, "finalize pass") == 0) {
      percent_done += finalize_pass;
      if (qa_calliper_track == 0)  // if QA has not been set, set mb_complete
	mb_complete = true;        // to true
    }
    else if (strcmp(c_str, "qa calliper") == 0) {
	float adjust = qa_calliper_track->done();
	if (adjust > 0.0)
	    percent_done += adjust;
	else
	    has_changed = false;
	mb_complete = true;
    }
    else if (strncmp(c_str, "qa_file_count:", 14) == 0) {
	initialize_qa_count(c_str);
    }
    else
	has_changed = false;

    delete c_str;
  }
  done = (unsigned short)((percent_done >= 0.0 && percent_done <= 100.0) ? floor(percent_done) : 100.0);
  return has_changed;
}

int ProgressMsgProcessor::extract_number_of_files(const char *str)
{
    int num_of_files = -1;
    string msg_str(str);
    int index = msg_str.find(':');
    if (index > 0) {
	index++;
	string num_str = msg_str.substr(index, msg_str.length()-index);
	int num = atoi(num_str.c_str());
	if (num >= 0) {
	    num_of_files = num;
	}
    }
    return num_of_files;
}

void ProgressMsgProcessor::initialize_update_count(const char* cstr)
{
    int num_of_files = extract_number_of_files(cstr);
    if (num_of_files >= 0) {
	update_pass_track->initialize_counts(num_of_files);
    }
}

void ProgressMsgProcessor::initialize_parse_count(const char* cstr)
{
    int num_of_files = extract_number_of_files(cstr);
    if (num_of_files >= 0) {
      parse_pass_track->initialize_counts(num_of_files);
    }
}

void ProgressMsgProcessor::initialize_qa_count(const char* cstr)
{
    int num_of_files = extract_number_of_files(cstr);
    if (num_of_files >= 0) {
	qa_calliper_track->initialize_counts(num_of_files);
    }
}

// ===========================================================================
TaskProgress::TaskProgress(const DOMString &name,
                       void *callback_object, void (*callback)(void *, double))
 : callback_object(callback_object),
   callback(callback),
   name(name),
   breakdown(NULL),
   steps(NULL),
   current_progress(0.0)
{
}

// ===========================================================================
// return whether handled
//
bool TaskProgress::process_breakdown_message(Message &msg) {
    bool handled = false;
    if (breakdown != NULL) {
        handled = breakdown->process_breakdown_message(msg);
    }
    else {
	DOMString what_for;
	msg.GetAttributeByName(forXMLStr, what_for);
	if (what_for.equals(name)) {
	    add_breakdown_or_steps(msg);
	    handled = true;
	}
    }
    return handled;
} 

// ===========================================================================
// return whether handled
//
bool TaskProgress::process_progress_message(Message &msg, DOMString &done_name) {
    bool handled = false;
    if (done_name.equals(name)) {
	callback(callback_object, 1.0);
	handled = true;
    }
    handled = handled || (breakdown != NULL && breakdown->process_progress_message(msg, done_name))
		      || (steps != NULL && steps->process_progress_message(msg, done_name));
    return handled;
} 

// ===========================================================================
// Call after determining that msg is a breakdown or steps for this
// particular task.
//
void TaskProgress::add_breakdown_or_steps(Message &msg) {
    DOMString tag;
    msg.GetTag(tag);
    if (breakdown == NULL && tag.equals(breakdownXMLStr)) {
	breakdown = new TaskBreakdown(*this, msg);
    }
    if (steps == NULL && tag.equals(stepsXMLStr)) {
	DOMString name;
	msg.GetAttributeByName(nameXMLStr, name);
        steps = new TaskSteps(name, *this, msg);
    }
}

// ===========================================================================
// Performs the callback if appropriate.
//
void TaskProgress::update_progress(double revision) {
    if (revision > 1.0) {
	revision = 1.0;
    }
    if (revision > current_progress) {
	current_progress = revision;
	if (callback != NULL && callback_object != NULL) {
	    callback(callback_object, current_progress);
        }
    }
}

// ===========================================================================
//
TaskProgress::~TaskProgress() {
    delete breakdown;
    delete steps;
}

// ===========================================================================
//
MiniXML::DOMString TaskProgress::get_name() const {
    return name;
}

// ===========================================================================
//
TaskBreakdown::TaskBreakdown(TaskProgress &for_task,
                             Message &msg)
 : for_task(for_task)
{
    int ch_enum;
    bool enum_flag = msg.EnumerateChildren(ch_enum);
    size = 0;
    if (enum_flag) {
	Message *child;
	for (;msg.GetNextChild(ch_enum, &child);) {
	    DOMString child_tag;
	    child->GetTag(child_tag);
	    if (child_tag.equals(taskXMLStr)) {
		DOMString name;
		child->GetAttributeByName(nameXMLStr, name);
		DOMString size_str;
		child->GetAttributeByName(sizeXMLStr, size_str);
		long child_size = 1;
		if (size_str.length() > 0) {
		    string num_str = trivialStringFromDOMString(size_str);
		    child_size = atol(num_str.c_str());
		}
		long subtask_start = size;
		long subtask_end = subtask_start + child_size;
		size = subtask_end;
		Subtask *task = new Subtask(name, *this, subtask_start, subtask_end);
		tasks.push_back(task);
	    }
	}
    }
}

// ===========================================================================
//
TaskBreakdown::~TaskBreakdown() {
    list<Subtask *>::iterator iter;
    for (iter = tasks.begin(); iter != tasks.end(); ++iter) {
	delete (*iter);
    }
}

// ===========================================================================
// static member function, called when a subtask makes an update
//
void TaskBreakdown::update(double val) {
    for_task.update_progress(val);
}

// ===========================================================================
// return whether handled
//
bool TaskBreakdown::process_breakdown_message(Message &msg) {
    bool handled = false;
    list<Subtask *>::iterator iter;
    for (iter = tasks.begin(); iter != tasks.end(); ++iter) {
	handled = handled || (*iter)->task.process_breakdown_message(msg);
    }
    return handled;
}

// ===========================================================================
// return whether handled
//
bool TaskBreakdown::process_progress_message(Message &msg, DOMString &done_name) {
    bool handled = false;
    list<Subtask *>::iterator iter;
    for (iter = tasks.begin(); iter != tasks.end(); ++iter) {
	handled = handled || (*iter)->task.process_progress_message(msg, done_name);
    }
    return handled;
}

// ===========================================================================
Subtask::Subtask(DOMString name, TaskBreakdown &parent, long start, long end)
 : name(name),
   parent(parent),
   start(start),
   end(end),
   task(name, (void *)this, callback)
{
}

// ===========================================================================
// static member function, called when a task makes an update
//
void Subtask::callback(void *obj, double val) {
    Subtask *subtask = (Subtask *)obj;
    subtask->note_progress(val);
}

// ===========================================================================
//
void Subtask::note_progress(double val) {
    // Map 0<=val<=1 to the range, based on start and end, and size of the entire breakdown.
    if (parent.size > 0){
        parent.update(((1-val)*start+val*end)/parent.size);
    }
}

// ===========================================================================
//
TaskSteps::TaskSteps(const DOMString &name, TaskProgress &for_task, Message &msg)
 : for_task(for_task),
   name(name)
{
    DOMString count_str;
    msg.GetAttributeByName(countXMLStr, count_str);
    long count = 1;
    if (count_str.length() > 0) {
	string count_str_2 = trivialStringFromDOMString(count_str);
	count = atol(count_str_2.c_str());
    }
    expected = count;
    current = 0;
}

// ===========================================================================
// return whether handled
//
bool TaskSteps::process_progress_message(Message &msg, DOMString &done_name) {
    if (done_name.equals(name)) {
	current += 1;
	double result = (double)current / (double)expected;
	for_task.update_progress(result < 1.0 ? result : 1.0);
	return true;
    }
    else {
	return false;
    }
}

//=========================================================================
//MsgGroupIdNode member functions

MsgGroupIdNode::MsgGroupIdNode(unsigned long mgid, MsgGroupIdNode *par,
			       const string &nm)
     : name(nm)
{
  msg_gid = mgid;
  parent = par;
  first_child = 0;
  sibling = 0;
  severity = normal_lp_sev;
}

MsgGroupIdNode::MsgGroupIdNode()
{
  msg_gid = 0;
  parent = 0;
  first_child = 0;
  sibling = 0;
  severity = normal_lp_sev;
}

MsgGroupIdNode::~MsgGroupIdNode()
{
  if (first_child)
    delete first_child;
  if (sibling)
    delete sibling;
}

void MsgGroupIdNode::add_new_mgid(DOMString& child, DOMString &child_name,
				  DOMString& par)
{
  unsigned long ch_mgid, parent_mgid;
  bool done = false;
  if (child.length() > 0) {
    char *ch_str = trivialFromDOMString(child);
    if (ch_str) {
      ch_mgid= atol(ch_str);
      char *par_str = (par.length() > 0) ? trivialFromDOMString(par) : 0;
      if (par_str)
	parent_mgid = atol(par_str);
      else
	parent_mgid = 0;
      char *ch_name_str = trivialFromDOMString(child_name);
      
      add_new_mgid(ch_mgid, ch_name_str, parent_mgid, done);

      if (par_str) delete par_str;
      delete ch_str;
      delete ch_name_str;
    }
  }
  if (!done)
    cerr << "Could not add new message group id to the tree." << endl;
}

void MsgGroupIdNode::add_new_mgid(unsigned long gid,
				  const string &name,
				  unsigned long parent_mgid,
				  bool& done)
{
  if (!done && msg_gid == parent_mgid) {
    MsgGroupIdNode *new_child = new MsgGroupIdNode(gid, this, name);
    if (first_child) {
      MsgGroupIdNode *next_ch = first_child;
      while (next_ch->sibling)
	next_ch = next_ch->sibling;
      next_ch->sibling = new_child;
    } else
      first_child = new_child;
    done = true;
  }
  if (!done && sibling)
    sibling->add_new_mgid(gid, name, parent_mgid, done);
  if (!done && first_child)
    first_child->add_new_mgid(gid, name, parent_mgid, done);
  return;
}

MsgGroupIdNode* MsgGroupIdNode::find_mgid_node(unsigned long mgid)
{
  MsgGroupIdNode *node = 0;
  if (mgid == msg_gid)
    node = this;
  if (!node && sibling)
    node = sibling->find_mgid_node(mgid);
  if (!node && first_child)
    node = first_child->find_mgid_node(mgid);
  return node;
}

void MsgGroupIdNode::reset()
{
    if (sibling)
	delete sibling;
    if (first_child)
	delete first_child;
    msg_gid = 0;
    parent = 0;
    first_child = 0;
    sibling = 0;
    severity = normal_lp_sev;
}

const string &MsgGroupIdNode::getName() const
{
    return name;
}

//========================================================================
//
lp_msg_sev severity_from_string(const char *sevstr)
{
  lp_msg_sev sev;
  if (!strcmp(sevstr, "error"))
    sev = error_lp_sev;
  else if (!strcmp(sevstr, "catastrophe"))
    sev = catastrophe_lp_sev;
  else if (!strcmp(sevstr, "warning"))
    sev = warning_lp_sev;
  else if (!strcmp(sevstr, "normal"))
    sev = normal_lp_sev;
  else if (!strcmp(sevstr, "unknown"))
    sev = explicit_unknown_lp_sev;
  else {
    sev = no_lp_sev;
  }
  return sev;
}

//========================================================================
//
const char *string_from_severity(lp_msg_sev sev)
{
    static const char *sevs[]
	= { "normal",
	    "warning",
	    "error",
	    "catastrophe",
	    "unknown",
	    ""
	  };
    if (sev < normal_lp_sev || no_lp_sev < sev) {
	sev = no_lp_sev;
    }
    return sevs[sev];
}

// ===========================================================================
//Go through command line options and initialize lpservice_name
//and passservice_name
static int initialize_service_names(int num_args, char **args)
{
  int i = 1;
  while (i < num_args) {
    if (strcmp("-lpservice", args[i]) == 0 ) {
      if (++i < num_args && strncmp(args[i], "-", 1) != 0)
	lpservice_name = args[i];
      else {
	cerr << "Invalid value for option -lpservice\n";
	break;
      }
    } 
    else if (strcmp("-passservice", args[i]) == 0 ) {
      if (++i < num_args && strncmp(args[i], "-", 1) != 0)
	passservice_name = args[i];
      else {
	cerr << "Invalid value for option -passservice\n";
	break;
      }
    }
    else if (strcmp("-trace_sends", args[i]) == 0) {
	trace_sends = true;
    }
    i++;
  }
  if ( lpservice_name.empty() || passservice_name.empty() ) {
    cerr << "Error: Usage: log_presentation -lpservice lpname -passservice passname" << endl;
    return 1;
  }
  return 0;
}

int main(int argc, char **argv)
{

  dbgprt("starting");

  // _asm int 3;

  if (initialize_service_names(argc, argv))
    return 1;

  // Initialize threads library.
  SAThreads::initialize();

  if (!initialize_filtersev()) {
      cerr << "failed to initialize filtersev" << endl;
  }

  IdPool idPool;

  // Start message processor thread. This thread waits till there is 
  // at least one message in the global list.
  Procedure *processor = new MsgProcessor(idPool);
  Thread msg_thread("message processor");
  msg_thread.start(*processor);

  // Start listening on socket, create a new thread for every "accept"
  // socket command. All the messages are placed in the global list of
  // messages.
  MessageReceiver rcvr(idPool);
  if (!listen(lpservice_name.c_str(), rcvr) )
    cerr << "Failed to listen on log presentation server " << lpservice_name.c_str() << endl;

  return 0;
}
