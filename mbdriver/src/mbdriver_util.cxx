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
#include "mbdriver_util.h"
#include "threads.h"
#include "svcnames.h"
#include "msg.h"
#include "mbuildmsg.h"
#include "transport.h"
#include "mbmsgser.h"
#include "minidom.h"

using namespace MBDriver;
using namespace std;
using namespace nTransport;
using namespace SAThreads;
using namespace MiniXML;

namespace MBDriver {

//==============================================================================
bool trace_sends = false;
bool trace_receives = false;

//==============================================================================
void dbgprt(const char *str) {
    // Don't use msg here, because it's for debugging.
    // Also, we're in mbdriver which initiates log_presentation for the
    // benefit of other processes communicating with the user.
    // This function is not for user meesages.
    cerr << str << endl << flush;
}

//==============================================================================
Mutex had_progress_mutex;
bool had_progress;
bool done_sent = false;

// ===========================================================================
ServicePort &lpservice_port()
{
    static ServicePort *service_p = new ServicePort(lpservice());
    return *service_p;
}

// ===========================================================================
// Sends a single message via log presentation.
//
void one_message(const string &msg, const string &severity) {
    Message xml_obj(trivialToDOMString("msg"));
    xml_obj.SetText(trivialToDOMString(msg));
    xml_obj.AddAttribute(trivialToDOMString("parent"), trivialToDOMString("0"));
    xml_obj.AddAttribute(trivialToDOMString("sev"), trivialToDOMString(severity));
    MessageSerializer serializer;
    SingleSender<Message> communicator(xml_obj, serializer);
    connect(lpservice_port(), communicator);
}

}

