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
#include "transport.h"
#include "guidriver.h"
#include "threads.h"
#include "sys/types.h"
#include <sys/stat.h>
#ifdef ISO_CPP_HEADERS
#include <iostream>
#include <sstream>
#else
#include <iostream.h>
#include <strstream.h>
#endif
#include "startprocess.h"
#include "mbuildmsg.h"
#include "mbmsgser.h"
#include "mbdriver_util.h"
#include "util.h"
#include "svcnames.h"
#include "lpdriver.h"
#include "xmlstrs.h"
#include "mbdriver.h"
#include "model_server_driver.h"
#include "model_server_utils.h"

using namespace MBDriver;
using namespace std;
using namespace nTransport;
using namespace MiniXML;
using namespace SAThreads;

// ===========================================================================
// Coordinate the use of the GUI connection by the log presentation
// communication thread.  The GUI communication thread might terminate,
// and at that point the log presentation had better stop using the
// GUI connection.
//
static Mutex connection_guard;
static Connection<Message> *connection = NULL;
static bool send_on_connection(Message &msg) {
    Exclusion e(connection_guard);
    if (connection != NULL) {
        return connection->send(msg);
    }
    return false;
}

// ===========================================================================
//
class GUIThread : public Procedure
{
public:
    GUIThread(MBDriver::pid_t gui_pid, Listener &l, Mutex &mutex)
      : guiPid(gui_pid),
	guiListener(l),
	mutex(mutex) {}
    virtual void run();
private:
    MBDriver::pid_t guiPid;
    Listener &guiListener;
    Mutex &mutex;
};

void
GUIThread::run() {
	// Just wait for the process to terminate.
	wait_process(guiPid);

	// Stop the listener, if there was no connection.
	// The presence of  indicates a connection was made.
	bool doit = false;
	{
	    doit = !log_presentation_thread_created();
	}
	if (doit) {
	    // Note: The GUI listening thread is terminated by shutting the
	    // guiListener.  If a connection occurs, it is the responsibility
	    // of the 'communicate' function; otherwise it is handled here.
	    // In the latter case, it is critical that no one else call 'shut'
	    // because the second call will find that the listener object is
	    // already deleted.
	    guiListener.shut();
	}
	delete this;
}


// ###########################################################################
// Threads that deal with the GUI and its socket.
// ===========================================================================
//
class ServerToGUI : public Communicator<Message> {
public:
    ServerToGUI(Thread &, const char *proj_name,
			  const char *proj_file,
			  bool auto_start_gui);
    ~ServerToGUI();
    virtual void communicate(Connection<Message> &);
    virtual void ready(Listener &);
    virtual Serializer<Message> *getSerializer()
	{return new MessageSerializer();}
    virtual void releaseSerializer(Serializer<Message> *serializer) 
	{ delete serializer; }
    typedef enum { inactive, started, killed } processState;

    bool heardGUI() { return log_presentation_thread_created(); } // created as soon as hear from GUI
private:
    processState mbProcessState;
    int mbSend; // for synchronization of messages to GUI.
    string *serverShutdownTime;
    string *qaTaskFile;
    string *projectPath;
    project_kind projectKind;
    string *outputPath;
    MBDriver::pid_t guiPid;
    Thread &guiThread;
    Mutex mutex;
    Listener *guiListener;
    const char *projectName;
    const char *projectFile;
    bool autoStartGui;
};

// ===========================================================================
//
ServerToGUI::ServerToGUI(Thread &gui_thread,
			 const char *proj_name, const char *proj_file,
			 bool auto_start_gui)
    : mbProcessState(inactive),
      serverShutdownTime(NULL),
      qaTaskFile(NULL),
      projectPath(NULL),
      projectKind(pk_unknown),
      outputPath(NULL),
      guiPid(null_pid),
      guiThread(gui_thread),
      guiListener(NULL),
      projectName(proj_name),
      projectFile(proj_file),
      autoStartGui(auto_start_gui)
{
}

// ===========================================================================
//
ServerToGUI::~ServerToGUI()
{
    delete serverShutdownTime;
    delete qaTaskFile;
    delete projectPath;
    delete outputPath;
}

// ===========================================================================
// After listening is established and before accepting connections,
// start the Java GUI process and the thread which tracks it.
//
void ServerToGUI::ready(Listener &listener)
{
    if (!autoStartGui) {
	// The Gui has been started manually.
	guiListener = &listener;
	return;
    }
    // Form the process filename and args.
    string javacmd;
    {
	// Look up CodeRover home directory.
	string rover_java;
	bool have_rover_java = false;
	bool rover_java_exists = false;
	if (psethome().length() > 0) {
	    have_rover_java = true;
	    rover_java = psethome();
#ifdef _WIN32
	    rover_java.append("\\jre\\bin\\java.exe");

	    // Determine whether that name refers to an extant file.
	    struct stat s;
	    int ret = stat(rover_java.c_str(), &s);
	    if (ret == 0 && (s.st_mode & _S_IFMT) != 0) {
		rover_java_exists = true;
	    }
#else
	    rover_java.append("/jre/bin/java");

	    // Determine whether that name refers to an extant file.
	    struct stat s;
	    int ret = stat(rover_java.c_str(), &s);
	    if (ret == 0 && (s.st_mode & S_IXUSR) != 0) {
		rover_java_exists = true;
	    }
#endif // _WIN32
	}
	if (rover_java_exists) {
	    javacmd = rover_java;
	}
	// If doesn't exist, attempt to set the string anyway, for the diagnostic.
	else if (have_rover_java) {
	    javacmd = rover_java;
	}
    }
    string classpath = psethome();
    classpath.append("/lib/ModelBuildGUI.jar");
    string home = psethome();

    string lmode;
#ifdef ISO_CPP_HEADERS
    stringstream lmodestr;
#else
    strstream lmodestr;
#endif
    lmodestr << get_license_mode();
#ifndef ISO_CPP_HEADERS
    lmodestr << ends;
#endif
    lmode = lmodestr.str();

    static char *argv[] = {
	"-jar",									// 0
	NULL, // CLASSPATH string               // 1
	"-service",                             // 2
	NULL,                                   // 3
	"-home",                                // 4
	NULL,                                   // 5
	"-lmode",								// 6
	NULL,									// 7
	"-projectFile",                         // 8
	NULL, // "d:/parsercmd/parsercmd.dsp",  // 9
	NULL };                                 // 10
    argv[1] = (char *)classpath.c_str();
    argv[3] = (char *)guiservice();
    argv[5] = (char *)home.c_str();
    argv[7] = (char *)lmode.c_str();
    if (projectFile == NULL) {
	argv[8] = NULL;
    }
    else {
	argv[9] = (char *)projectFile;
    }

    guiPid = start_process(javacmd.c_str(), argv, 0);
    if (guiPid == null_pid) {
	cerr << javacmd.c_str() 
	    << " " << argv[0] 
	    << " " << argv[1]
	    << " " << argv[2]
	    << " " << argv[3]
	    << " " << argv[4]
	    << " " << argv[5]
	    << " " << argv[6]
	    << " " << argv[7];
	if (argv[8] != NULL) {
	    cerr << " " << argv[8] << " " << argv[9];
	}
	cerr << endl;
	cerr << "Failed to start the GUI." << endl;
        listener.shut();
    }
    else {
	guiListener = &listener;
	Procedure *guiProcedure = new GUIThread(guiPid, listener, mutex);
	guiThread.start(*guiProcedure);
    }
}

// ===========================================================================
//
void ServerToGUI::communicate(Connection<Message> &conn)
{
    // Ensure only one socket is attempting to communicate,
    // and start the Log Presentation thread and process.
    bool isAGUIConnection = false;
    bool had_start = false;
    bool log_presentation_started = false;
    bool had_good_null_message = false;
    bool had_null_message = false;

    // wait for start command 
    char *recv_buf = NULL;
    int	 len	   = 0;

    for (;;) {
        Message *msg = conn.receive();
	if (msg == NULL && !conn.isError()) {
	    had_good_null_message = true;
	    had_null_message = true;
	    break;
	}
        if (msg == NULL) {
	    had_null_message = true;
	    break;
	}
	if (1) {
	  MessageSerializer s;
	  const char *data;
	  size_t length;
	  s.serialize(*msg, data, length);
#ifdef ISO_CPP_HEADERS
	  ostringstream strs;
#else
	  ostrstream strs;
#endif
	  strs << "driver received from GUI: ";
	  for (int i = 4; i < length; i += 1) {
	      strs << data[i];
	  }
#ifndef ISO_CPP_HEADERS
	  strs << ends;
#endif
	  string strs_str = strs.str();
	  dbgprt(strs_str.c_str());
	}

	DOMString tag;
	msg->GetTag(tag);
	if (!isAGUIConnection) {
	    isAGUIConnection = true;
	    {
		Exclusion s(mutex, "testing lp creation");
		if (log_presentation_thread_created()) {
		    // The already-existing PresentationListenerThread is
		    // associated with a different GUI communication thread.
		    // This thread is spurious.
		    return;
		}
	    }
	    // Log presentation runs concurrently with the GUI, rather than with
	    // a rebuild of the model, because it mediates what's visible to the
	    // user.
	    {
		Exclusion e(connection_guard);
		connection = &conn;
	    }
	    start_log_presentation(&send_on_connection);

	    // If we didn't auto-start the GUI,
	    // then there's no tracking thread running.
	    // Lock the Thread object to force the "listener" thread to wait.
	    if (!autoStartGui){
	        guiThread.lock();
	    }
	    
	    // Close the listener now, to take care of that thread.
	    // Communications will continue in this thread, on the connected socket.
	    guiListener->shut();
	}

	if ((tag.equals(buildXMLStr) || tag.equals(startXMLStr))
	    && projectPath != NULL && outputPath != NULL
	    && projectKind != pk_unknown) {

	    string errmsg;
	    if( !check_lm_license_file(errmsg) ) {
		Message *xmsg = new Message((DOMString(codeRoverFailedXMLStr)));
		if (xmsg) {
		    DOMString emsg;
		    trivialToDOMString(emsg, errmsg.c_str());
		    xmsg->SetText(emsg);
		    conn.send(*xmsg);
		    delete xmsg;
		}
	    }
	    else {
		log_presentation_started = true;
		if (had_start) {
		    // This is a subsequent model build start.
		    // Reset the log presentation.
		    {
			Exclusion s(had_progress_mutex, "resetting lp progress");
			had_progress = false;
			done_sent = false;
		    }
		    Message msg(static_cast<DOMString>(progressXMLStr));
		    msg.AddAttribute(doneXMLStr, invokeModelBuildXMLStr);
		    MessageSerializer serializer;
		    SingleSender<Message> sender(msg, serializer);
		    if ( !connect(lpservice_port(), sender) )
			cerr << "Could not send xml progress message, failed connecting to log presentation server: " << lpservice() << endl;
		    else if (!sender.success())
			cerr << "Failure while sending xml progress message to log presentation server: " << lpservice() << endl;
		}
		shut_down_model_server(projectPath, outputPath);

		start_model_build_process(*projectPath, *outputPath,
                                          qaTaskFile, projectKind, send_on_connection);
		had_start = true;
	    }
	}
	else if (tag.equals(commandXMLStr)) {
	    DOMString cmd;
	    msg->GetText(cmd);
	    vector<string> argv;
	    int ch_enum;
	    bool enum_flag = msg->EnumerateChildren(ch_enum);
	    if (enum_flag) {
		Message *child;
		for (;msg->GetNextChild(ch_enum, &child);) {
		    DOMString child_tag;
		    child->GetTag(child_tag);
		    if (child_tag.equals(argXMLStr)) {
			DOMString arg;
			child->GetText(arg);
			argv.push_back(trivialStringFromDOMString(arg));
		    }
      		}
	    }

	    log_presentation_started = true;
	    if (had_start) {
		// This is a subsequent model build start.
		// Reset the log presentation.
		{
		    Exclusion s(had_progress_mutex, "resetting lp progress");
		    had_progress = false;
		    done_sent = false;
		}
		Message msg(static_cast<DOMString>(progressXMLStr));
		msg.AddAttribute(doneXMLStr, invokeModelBuildXMLStr);
		MessageSerializer serializer;
		SingleSender<Message> sender(msg, serializer);
		if ( !connect(lpservice_port(), sender) )
		    cerr << "Could not send xml progress message, failed connecting to log presentation server: " << lpservice() << endl;
		else if (!sender.success())
		    cerr << "Failure while sending xml progress message to log presentation server: " << lpservice() << endl;
	    }
	    start_model_build_process(trivialStringFromDOMString(cmd), argv, send_on_connection);
	    had_start = true;
	}
	else if (tag.equals(dspXMLStr) || tag.equals(clearmakeXMLStr)) {
	    DOMString pathstr;
	    msg->GetText(pathstr);
	    char *pathchars = trivialFromDOMString(pathstr);
	    delete projectPath;
	    projectPath = new string(pathchars);
	    if (tag.equals(clearmakeXMLStr)) {
	        projectKind = pk_clearmake;
	    }
	    else {
// This ifdef is poor.  GUI or pbmb_driver.pl should determine project kind,
// i.e. pk_defer.  But pbmb_driver isn't ready for that yet.
#ifdef _WIN32
	        projectKind = pk_from_extension_dflt_dsp;
#else
	        projectKind = pk_defer;
#endif
	    }
	    delete [] pathchars;
	}
	else if (tag.equals(outputXMLStr)) {
	    DOMString os;
	    msg->GetText(os);
	    char *p = trivialFromDOMString(os);
	    delete outputPath;
	    outputPath = new string(p);
	    delete [] p;
	}
	else if (tag.equals(qataskXMLStr)) {
	    DOMString task;
	    msg->GetText(task);
	    char *t = trivialFromDOMString(task);
	    delete qaTaskFile;
	    qaTaskFile = new string(t);
	    delete [] t;
	}
	else if (tag.equals(autoshutdownXMLStr) ){
	    DOMString time;
	    msg->GetText(time);
	    char *t = trivialFromDOMString(time);
	    delete serverShutdownTime;
	    serverShutdownTime = new string(t);
	    delete [] t;
	}
	else if (tag.equals(startPmodServerXMLStr) && outputPath != NULL ) {
	    if (!(log_presentation_started
                       =    log_presentation_started
                         || wait_for_log_presentation_service(3 /* seconds */))) {
		Message *xmsg = new Message((DOMString(codeRoverFailedXMLStr)));
		if (xmsg) {
		    xmsg->SetText(trivialToDOMString("Internal error: Log presentation is not running."));
		    conn.send(*xmsg);
		    delete xmsg;
		}
	    }
	    else {
		start_model_server_process(*projectPath, *outputPath, conn, 
		                           serverShutdownTime);
	    }
	}
	else if (tag.equals(cancelXMLStr)) {
	    cancel_model_build_process();
	}
    }

    if (isAGUIConnection) {
	// Done communicating with the GUI. Presumably it has shut down OK.
	bool ds;
	{
	    Exclusion s(had_progress_mutex, "checking done sent");
	    ds = done_sent;
	}
	cerr << "Done communicating with the GUI, due to "
		<< (had_good_null_message ? (ds ? "null message upon completion" : "null message") : had_null_message ? "error" : "something")
		<< "." << endl << flush;

	if (ds) {
	    // Normal closing of the GUI after completion of the command.
	    // Don't terminate anything in the process group.
	}
	else {
	    // Early closing of the GUI terminates the command.
	    shut_down_model_build_process();
	}

	//set variable so that pmod_server threads reads it and terminates
	//pmod_server if code rover browser has not been started yet.
	terminate_model_server();

	shut_down_log_presentation();

	// !!! Should still check that _all_ threads are terminated
	// decently before exiting.
	{
	    Exclusion e(connection_guard);
	    connection = NULL;
	}
	if (!autoStartGui){
	    guiThread.unlock();
	}
    }
}



// ===========================================================================
//
class GUIListenerThread : public Procedure
{
public:
    GUIListenerThread(const char *proj_name, const char *proj_file,
		      bool auto_start_gui)
	: projectName(proj_name), projectFile(proj_file), 
	   Procedure(), autoStartGui(auto_start_gui),
	   guiThread("GUIThread") {}
    virtual void run();
private:
    Thread guiThread;
    const char *projectName;
    const char *projectFile;
    bool autoStartGui;
};

void
GUIListenerThread::run() {
	// The ServerToGUI can't be deleted, because we don't track the
	// termination of each thread which uses it.  There's no particular
	// need to delete it because only 1 gets allocated per GUI,
	// and only 1 GUI is initiated by mbdriver.
	ServerToGUI *gui_svr = new ServerToGUI(guiThread, projectName,
							  projectFile,
							  autoStartGui);
	int success = listen(guiservice(), *gui_svr);
	if (!success) {
	    fprintf(stderr, "Socket failed to listen for GUI with service %s.\n", guiservice());
	}
	// Wait until the GUI tracking thread is finished.
	guiThread.lock();
	if (!gui_svr->heardGUI()) {
	    fprintf(stderr, "GUI failed to make communications connection with driver.\n");
	}
	delete this;
}

// ===========================================================================
//
void MBDriver::run_gui_process(const char *proj_name, 
			       const char *proj_file,
			       bool auto_start_gui)
{
    Procedure *gui_procedure = new GUIListenerThread(proj_name, proj_file,
					             auto_start_gui);
    Thread gui("GUI listener");
    gui.start(*gui_procedure);

    // Wait until the GUI service is finished.
    gui.lock();
}
