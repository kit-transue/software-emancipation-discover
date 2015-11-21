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
#include "mbmsgser.h"
#include "msgreceiver.h"
#include "logpresent.h"
#include "mbuildmsg.h"
#ifdef ISO_CPP_HEADERS
#include <sstream>
#else
#include <strstream.h>
#endif

using namespace MBDriver;
using namespace nTransport;
using namespace MiniXML;

static const XMLCh breakdownXMLStr[] =
{
	chLatin_b, chLatin_r, chLatin_e, chLatin_a, chLatin_k, chLatin_d, chLatin_o, chLatin_w, chLatin_n, chNull
};
static const XMLCh msgXMLStr[] =
{
        chLatin_m, chLatin_s, chLatin_g, chNull
};
static const XMLCh normalXMLStr[] =
{
        chLatin_n, chLatin_o, chLatin_r, chLatin_m, chLatin_a, chLatin_l, chNull
};
static const XMLCh spawnXMLStr[] =
{
        chLatin_s, chLatin_p, chLatin_a,chLatin_w, chLatin_n, chNull
};
static const XMLCh stepsXMLStr[] =
{
	chLatin_s, chLatin_t, chLatin_e, chLatin_p, chLatin_s, chNull
};
static const XMLCh progressXMLStr[] =
{
        chLatin_p, chLatin_r, chLatin_o, chLatin_g, chLatin_r, chLatin_e, chLatin_s, chLatin_s, chNull
};
static const XMLCh taskXMLStr[] =
{
	chLatin_t, chLatin_a, chLatin_s, chLatin_k, chNull
};
static const XMLCh getXMLStr[] =
{
        chLatin_g, chLatin_e, chLatin_t, chNull
};
static const XMLCh emptyXMLStr[] =
{
        chNull
};
static const XMLCh oneXMLStr[] =
{
        chDigit_1, chNull
};
static const XMLCh valXMLStr[] =
{
        chLatin_v, chLatin_a, chLatin_l, chNull
};
static const XMLCh mgidXMLStr[] =
{
    chLatin_m, chLatin_g, chLatin_i, chLatin_d, chNull
};
static const XMLCh sevXMLStr[] =
{
    chLatin_s, chLatin_e, chLatin_v, chNull
};
static const XMLCh errorXMLStr[] =
{
    chLatin_e, chLatin_r, chLatin_r, chLatin_o, chLatin_r, chNull
};

extern MessageList messages_list;
void MessageReceiver::communicate(Connection<Message>& conn) 
{
    DOMString *msg_example = NULL;
    for (;;) {
	Message *obj = conn.receive();
	if (obj) {
	    DOMString tag;
	    obj->GetTag(tag);
	    if (tag.equals(msgXMLStr) || tag.equals(spawnXMLStr) || tag.equals(progressXMLStr)
	        || tag.equals(taskXMLStr) || tag.equals(stepsXMLStr) || tag.equals(breakdownXMLStr)) {
		Message *msg = new Message(*obj);
		messages_list.addMessage(msg);
	    }
	    else if (tag.equals(mgidXMLStr)) {
		DOMString attr_val;
		if (obj->GetAttributeByName(getXMLStr, attr_val)
			&& (attr_val.equals(emptyXMLStr) || attr_val.equals(oneXMLStr))) {
		    unsigned long id = idPool.get();
		    Message reply = DOMString(mgidXMLStr);
#ifdef ISO_CPP_HEADERS
		    stringstream id_stream;
#else
		    strstream id_stream;
#endif
		    id_stream << id;
#ifndef ISO_CPP_HEADERS
		    id_stream << ends;
#endif
		    string id_string = id_stream.str();
		    DOMString idDOMStr;
		    trivialToDOMString(idDOMStr, id_string.c_str());
		    reply.AddAttribute(valXMLStr, idDOMStr);
		    conn.send(reply);
		}
	    }
	    if (tag.equals(msgXMLStr)) {
	        if (msg_example == NULL) {
		    msg_example = new DOMString;
		}
		obj->GetText(*msg_example);
	    }
	}
	else {
	      if (!conn.isError()) {
		  // possibly a connection from the name server,
		  // or the connected component has no more messages to send
	      }
	      else {
		  Message *msg = new Message(DOMString(msgXMLStr));
		  DOMString text;
		  trivialToDOMString(text,
		      msg_example == NULL ?
			  "Error on socket, probably a failure to close, at opposite end of socket."
			  : "Process at opposite end of socket failed to close it. Preceding message was $1.");
		  msg->AddText(text);
		  msg->AddAttribute(sevXMLStr, normalXMLStr);

		  if (msg_example != NULL) {
		      Message child(trivialToDOMString("arg"));
		      child.SetText(*msg_example);
		      child.AddAttribute(trivialToDOMString("num"), trivialToDOMString("1"));
		      msg->AddChild(child);
		  }

		  messages_list.addMessage(msg);
	      }
	      break;
	}
    }
    delete msg_example;
    return;
}

Serializer<Message>* MessageReceiver::getSerializer() 
{
  return new MessageSerializer();
}

void MessageReceiver::releaseSerializer(Serializer<Message>* serializer) 
{
  delete serializer;
}

