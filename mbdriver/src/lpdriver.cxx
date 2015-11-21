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
#include "lpdriver.h"
#include "msg.h"
#include "threads.h"
#include "transport.h"
#include "mbuildmsg.h"
#include "mbmsgser.h"
#include "mbdriver_util.h"
#include "util.h"
#include "svcnames.h"
#include "xmlstrs.h"

#ifdef ISO_CPP_HEADERS
#include <sstream>
#else
#include <strstream.h>
#endif

using namespace MBDriver;
using namespace std;
using namespace nTransport;
using namespace MiniXML;
using namespace SAThreads;

static Thread *lp_thread = NULL;
static Listener *lplistener = NULL;
static MBDriver::pid_t lppid = null_pid;
static bool server_ready = false;
static Monitor ready_monitor;

// ===========================================================================
// Threads that deal with the Log Presentation and its socket.
// ===========================================================================
//
class ServerToPresentation : public Communicator<Message> {
public:
    ServerToPresentation(bool guiconn(Message &), Listener *&lpl, MBDriver::pid_t &lppid);
    virtual void communicate(Connection<Message> &);
    virtual void ready(Listener &);
    virtual Serializer<Message> *getSerializer()
	{return new MessageSerializer();}
    virtual void releaseSerializer(Serializer<Message> *serializer) 
	{ delete serializer; }
    typedef enum { inactive, started, killed } processState;
    ~ServerToPresentation();
private:
    processState lpProcessState;
    int lpSync;
    Mutex mutex;
    bool (*sendToGUI)(Message &);
    MBDriver::pid_t presentationPid;
    Listener *&lpListener;
    MBDriver::pid_t &lpPid;
};

// ===========================================================================
//
ServerToPresentation::ServerToPresentation(bool guiconn(Message &),
					   Listener *&lpl,
					   MBDriver::pid_t &lppid)
 : lpProcessState(inactive),
   lpSync(0),
   sendToGUI(guiconn),
   presentationPid(null_pid),
   lpListener(lpl),
   lpPid(lppid)
{
}

// ===========================================================================
//
ServerToPresentation::~ServerToPresentation()
{
    lpListener = NULL;
}

// ===========================================================================
// Start the log presentation process.
// Do this here rather from the Perl Script, so that waiting until
// the log presentation is ready before starting the Perl script,
// can be handled in the C++ code.  The log presentation listener
// can, once it has the connection, enable the model build to go
// ahead.
//
void ServerToPresentation::ready(Listener &listener)
{
    // Form the process filename and args.
    string lpcmd = psethome();
    lpcmd.append("/bin/log_presentation");
    static char *argv[] = { 
	"-lpservice",
	NULL,
	"-passservice",
	NULL,
	NULL,
	NULL};
    argv[1] = (char *)lpservice();
    argv[3] = (char *)passservice();
    if (trace_sends) {
	argv[4] = "-trace_sends";
    }
    {
	Exclusion s(had_progress_mutex, "init progress starting lp");
	had_progress = false;
	done_sent = false;
    }
    presentationPid = start_process(lpcmd.c_str(), argv, 0);
    if (presentationPid == null_pid) {
	msg("Failed to start the log presentation.", catastrophe_sev) << eom;
	listener.shut();
	lpListener = NULL;
    }
    else {
	lpListener = &listener;
    }
    lpPid = presentationPid;
#if 0
    init_lp_service(lpservice(), "0");
#endif
    ready_monitor.lock();
    server_ready = true;
    ready_monitor.notify();
    ready_monitor.unlock();
}

// ===========================================================================
//
void ServerToPresentation::communicate(Connection<Message> &conn)
{
    // Allow connections from the name server which send no messages.
    // One other connection becomes dedicated to the log presentation.
    // Any more could cause synchronization problems.
    bool amForLogPresentation = false;
    for (;;) {
	Message *msg = conn.receive();
	if (msg == NULL) {
	    string errmsg = (conn.isError() ? "Erroneous" : "Null");
	    errmsg += " message on server to log presentation; terminating ";
	    errmsg += (amForLogPresentation ? "messaging socket" : "socket at first contact");
	    errmsg += ".";
	    if (amForLogPresentation || conn.isError() || trace_receives) {
		dbgprt(errmsg.c_str());
	    }
	    break;
	}
	if (trace_receives) {
	  MessageSerializer s;
	  const char *data;
	  size_t length;
	  s.serialize(*msg, data, length);
#ifdef ISO_CPP_HEADERS
	  ostringstream strs;
#else
	  ostrstream strs;
#endif
	  strs << "driver received: ";
	  for (int i = 4; i < length; i += 1) {
	      strs << data[i];
	  }
#ifndef ISO_CPP_HEADERS
	  strs << ends;
#endif
	  string strs_str = strs.str();
	  dbgprt(strs_str.c_str());
	}
	// Ensure only one socket is attempting to communicate.
	if (!amForLogPresentation) {
	    Exclusion s(mutex, "lpSync");
	    if (lpSync > 0) {
		cerr << "Rejecting a second connection to passthrough service." << endl << flush;
		return;
	    }
	    amForLogPresentation = true;
	    lpSync = 1;
	}
	DOMString tag;
	if ((msg->GetTag(tag), tag.equals(progressXMLStr)
	                       || tag.equals(messageXMLStr))) {
	    Exclusion s(had_progress_mutex, "track actual progress");
	    had_progress = true;
	}
	if (tag.equals(doneXMLStr)) {
	    Exclusion s(had_progress_mutex, "track done sent");
	    done_sent = true;
	}

	// logged message and what the GUI expects.
	bool send_ok = sendToGUI(*msg);
	if (trace_sends) {
	  MessageSerializer s;
	  const char *data;
	  size_t length;
	  s.serialize(*msg, data, length);
#ifdef ISO_CPP_HEADERS
	  ostringstream strs;
#else
	  ostrstream strs;
#endif
	  if (send_ok) {
	    strs << "driver sending: ";
	  }
	  else {
	    strs << "driver sending failure: ";
	  }
	  for (int i = 4; i < length; i += 1) {
	      strs << data[i];
	  }
#ifndef ISO_CPP_HEADERS
	  strs << ends;
#endif
	  string strs_str = strs.str();
	  dbgprt(strs_str.c_str());
	}
	if (!send_ok) {
	    break;
	}
    }
    if (amForLogPresentation) {
	// The connection is closed. Presumably the presentation process
	// has exited.
	presentationPid = null_pid;
    }
}

// ===========================================================================
//
class PresentationListenerThread : public Procedure
{
public:
    PresentationListenerThread(bool guiconn(Message &),
			       Listener *&lplistener,
			       MBDriver::pid_t &lppid)
                : sendToGUI(guiconn),
		  lpListener(lplistener),
		  lpPid(lppid) {}
    virtual void run();
private:
    bool (*sendToGUI)(Message &);
    Listener *&lpListener;
    MBDriver::pid_t &lpPid;
};

void
PresentationListenerThread::run() {
	ServerToPresentation *presentation_svr
	    = new ServerToPresentation(sendToGUI, lpListener, lpPid);
	int success = nTransport::listen(passservice(), *presentation_svr);
	if (!success) {
	    msg("Socket failed to listen to log presentation.", catastrophe_sev) << eom;
	}
	delete this;
}

// ===========================================================================
//
void MBDriver::start_log_presentation(bool guiconn(Message &))
{
    Procedure *lp_procedure = new PresentationListenerThread(guiconn,
					                     lplistener,
					                     lppid);
    lp_thread = new Thread("lp listener");
    lp_thread->start(*lp_procedure);
}

// ===========================================================================
//
static void ensure_server_ready()
{
    ready_monitor.lock();
    while (!server_ready) ready_monitor.wait();
    ready_monitor.unlock();
}

// ===========================================================================
//
bool MBDriver::wait_for_log_presentation_service(int max_seconds)
{
    ensure_server_ready();

    ServiceName lp_service_name(lpservice());
    int slept = 0;
    int port;
    int ipaddr;
    while (!lp_service_name.getPortAndAddr(port, ipaddr)) {
	// Check if log presentation is still alive.
	if (!is_process_alive(lppid)) {
	    cerr << "Failed to contact log presentation service. Log presentation process is not alive." << endl << flush;
	    return false;
	}
	if (max_seconds <= slept) {
	    cerr << "Log presentation service not found in " << max_seconds << " seconds." << endl << flush;
	    return false;
	}
	sleep_in_seconds(1);
	slept += 1;
    }
    cerr << "Contacted log presentation service.  Log presentation process is"
            << (is_process_alive(lppid) ? " " : " not ")
            << "alive." << endl << flush;
    return true;
}

// ===========================================================================
//
bool MBDriver::log_presentation_thread_created()
{
    return lp_thread != NULL;
}

// ===========================================================================
// Shut down log presentation, returning once it has stopped.
//
void MBDriver::shut_down_log_presentation()
{
    // This checks for shutting down Log Presentation in a primitive way.
    Listener *lpl = lplistener;
    if (lpl != NULL) {
	lpl->shut();
    }
    terminate_process(lppid);

    // Wait until the Log Presentation service is finished.
    if (lp_thread != NULL) {
	Exclusion lp_waiter(*lp_thread, "waiting for lp process");
    }
}

// ===========================================================================
//
static void wait_for_log_presentation_done()
{
    ensure_server_ready();
    wait_process(lppid);
}

// ===========================================================================
//
static bool guiconn_substitute(MBDriver::Message &msg) {
    return true;
}

// ===========================================================================
// Typical usage is to run these in separate windows:
//	mbdriver.exe -test_no_gui -log_windows -trace_sends
//	BuildModel -vlpsm/TODAY -mfull -lpservice Build:lp8676
//
void MBDriver::run_log_presentation_test()
{
    start_log_presentation(guiconn_substitute);
    cerr << "Started log presentation with lpservice " << lpservice() << endl << flush;
    wait_for_log_presentation_done();
}
