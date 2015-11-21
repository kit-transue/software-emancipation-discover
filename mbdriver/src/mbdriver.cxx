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
#include "mbdriver.h"
#include "threads.h"
#include "transport.h"
#include "mbuildmsg.h"
#include "startprocess.h"
#include "mbdriver_util.h"
#include "util.h"
#include "xmlstrs.h"
#include "mbmsgser.h"
#include "lpdriver.h"
#include "lpinterface.h"
#include "svcnames.h"
#include "msg.h"

using namespace MBDriver;
using namespace std;
using namespace nTransport;
using namespace MiniXML;
using namespace SAThreads;

static Thread *builder_thread = 0;
static Mutex mb_guard;
static MBDriver::pid_t mb_pid = null_pid;
static bool terminated = false;

// ===========================================================================
//
class MBThread : public Procedure
{
public:
    MBThread(MBDriver::pid_t &mb_pid, const char *cmd, char *argv[],
	     bool (*send2gui)(Message &));
    ~MBThread();
    virtual void run();
private:
    MBDriver::pid_t &mbPid;
    char *cmd;
    char **args;
    bool (*send2gui)(Message &);
};

MBThread::MBThread(MBDriver::pid_t &mb_pid, const char *cmdp, char *argv[], bool (*send2gui)(Message &))
  : mbPid(mb_pid), send2gui(send2gui)
{
    // cerr << "constructing MBThread" << endl;
    cmd = new char[strlen(cmdp) + 1];
    strcpy(cmd, cmdp);
    // Copy the args, first counting them.
    char **p;
    int count = 0;
    for (p = argv; *p != NULL; p += 1) {
	count += 1;
    }
    // cerr << "count is " << count << endl;
    args = new char *[count + 1];
    count = 0;
    for (p = argv;; p += 1) {
	// cerr << "adding arg number " << count << endl;
	if (*p == NULL) {
	    args[count] = NULL;
	    break;
	}
	args[count] = new char [strlen(*p) + 1];
	strcpy(args[count], *p);
	count += 1;
    }
    // cerr << "constructed MBThread" << endl;
}

MBThread::~MBThread()
{
    delete [] cmd;
    // Delete the args.
    for (char **p = args; *p != NULL; p += 1) {
	delete [] (*p);
    }
    delete [] args;
}

// Find the file name portion of file path.
// Path must not be quoted, nor end in a slash.
// Returns a pointer into the same string.
static char *fname(char *path) {
    char *p;
    return (p = strrchr(path, '/')) != NULL ? p+1 :
	   (p = strrchr(path, '\\')) != NULL ? p + 1 : path;
}

void
MBThread::run() {
    int emergency_id = 100000000;

    // Wait for log presentation.
    if (!wait_for_log_presentation_service(300 /* seconds */)) {
	Message xmsg = Message(DOMString(failedXMLStr));
	xmsg.SetText(trivialToDOMString("Internal error: Log presentation did not start."));
	send2gui(xmsg);

	delete this;
	return;
    }

    // Determine command name to use in diagnostics.
    char *cmd_name = fname(cmd);
    if (strncmp(cmd_name, "disperl", 7) == 0 && args[0] != NULL) {
        // Interpreting another command.
        cmd_name = fname(args[0]);
    }

    // Start the model build process.
    msg("Starting $1.", normal_sev) << cmd_name << eom;
    const int max_tries = 2;
    int count;
    int exit_code = 0;
    MBDriver::pid_t local_mbPid = null_pid;
    for (count = 0; count < max_tries; count += 1) {
	mb_guard.lock();
	if (terminated) {
	    mb_guard.unlock();
	    break;
	}
	local_mbPid = start_process(cmd, args, 2);
	if (local_mbPid == null_pid) {
	    mb_guard.unlock();
	    msg("Failed to start $1.", catastrophe_sev) << cmd_name << eoarg << eom;
	    // It's an emergency; try to send it via log_presentation also.
	    string fail_msg("Failed to start ");
	    fail_msg += cmd_name;
	    fail_msg += ".";
	    one_message(fail_msg.c_str(), "catastrophe");
	    break;
	}

	// Wait for the process to terminate.
	mbPid = local_mbPid;
	mb_guard.unlock();
	exit_code = wait_process(local_mbPid);

	bool trouble = false;
	{
	    Exclusion s(had_progress_mutex, "check on progress");
	    trouble = !had_progress;
	}
	if (!trouble) {
	    break;
	}
	sleep_in_seconds(2);
	{
	    Exclusion s(had_progress_mutex, "check on progress again");
	    trouble = !had_progress;
	}
	if (!trouble) {
	    break;
	}

	if (exit_code != 0) {
	    msg("Starting model build: process exited with code $1.", error_sev)
		<< exit_code << eoarg << eom;
	    // Send persistence message direct to GUI, to avoid being interpreted as progress.
	    string code_msg("Attempt to spawn child process failed with exit code ");

	    Message send_obj1 = Message(DOMString(messageXMLStr));
	    char buf[25];
	    sprintf(buf, "%lu", emergency_id++);
	    DOMString dom_buf;
	    trivialToDOMString(dom_buf, buf);
	    send_obj1.AddAttribute(nodeXMLStr, dom_buf);
	    send_obj1.AddAttribute(helpXMLStr, zeroXMLStr);
	    send_obj1.AddAttribute(parentXMLStr, zeroXMLStr);
	    send_obj1.AddAttribute(severityXMLStr, errorXMLStr);
	    sprintf(buf, "%d", exit_code);
	    (code_msg += buf) += ".";
	    send_obj1.SetText(trivialToDOMString(code_msg.c_str()));
	    send2gui(send_obj1);
	}
	if (exit_code == 0) {
	    msg("Starting model build: retry.", warning_sev) << eom;
	    // Send persistence message direct to GUI, to avoid being interpreted as progress.
	    string again_msg("Attempt to start ");
	    (again_msg += cmd_name) += " failed early; trying again.";

	    Message send_obj = Message(DOMString(messageXMLStr));
	    char buf[25];
	    sprintf(buf, "%lu", emergency_id++);
	    DOMString dom_buf;
	    trivialToDOMString(dom_buf, buf);
	    send_obj.AddAttribute(nodeXMLStr, dom_buf);
	    send_obj.AddAttribute(helpXMLStr, zeroXMLStr);

	    send_obj.AddAttribute(parentXMLStr, zeroXMLStr);
	    send_obj.AddAttribute(severityXMLStr, warningXMLStr);
	    send_obj.SetText(trivialToDOMString(again_msg.c_str()));
	    send2gui(send_obj);
	}
	{
	    // Prepare command string in a writable buffer.
	    list<string> arg_list;
	    arg_list.push_back(cmd);
	    for (int i = 0;; i += 1) {
		const char *arg = args[i];
		if (arg == NULL) {
		    break;
		}
		arg_list.push_back(arg);
	    }

	    string cmd_msg("Full command: ");
	    cmd_msg += prepare_command_arguments(arg_list);

	    Message send_obj = Message(DOMString(messageXMLStr));
	    char buf[25];
	    sprintf(buf, "%lu", emergency_id++);
	    DOMString dom_buf;
	    trivialToDOMString(dom_buf, buf);
	    send_obj.AddAttribute(nodeXMLStr, dom_buf);
	    send_obj.AddAttribute(helpXMLStr, zeroXMLStr);

	    send_obj.AddAttribute(parentXMLStr, zeroXMLStr);
	    send_obj.AddAttribute(severityXMLStr, normalXMLStr);
	    send_obj.SetText(trivialToDOMString(cmd_msg.c_str()));
	    send2gui(send_obj);
	}
	if (exit_code != 0) {
	    break;
	}

	// Now try again.  Better luck this time?
    }
    if (exit_code != 0) {
	msg("Child process returned code $1", catastrophe_sev) << exit_code << eoarg << eom;
    }
    if (count >= max_tries) {
	msg("$1 retries exhausted.", catastrophe_sev) << cmd_name << eoarg << eom;
    }
    else if (local_mbPid == null_pid) {
	// already reported
    }
    else {
        msg("$1 done.", normal_sev) << cmd_name << eoarg << eom;
    }

    // Wait a second before sending COMMAND_DONE_TEXT message
    // to log_presentation. This will, hopefully, give time for
    // the executed process to send all messages to log_presentation
    // before mbdriver sends a "done" message.
    sleep_in_seconds(1);

    // send done message via Log Presentation
    Message message = DOMString(msgXMLStr);
    DOMString doneText;
    trivialToDOMString(doneText, COMMAND_DONE_TEXT);
    DOMString cmdText;
    trivialToDOMString(cmdText, cmd_name);
    doneText += cmdText;
    message.SetText(doneText);
    MessageSerializer ser;
    SingleSender<Message> sndr(message, ser);
    connect(lpservice_port(), sndr);

    delete this;
}

// ===========================================================================
//
void MBDriver::start_model_build_process(string dspPath,
					 string outputPath,
					 string *qaTaskFile,
					 project_kind projectKind,
					 bool send2gui(Message &msg))
{
    // Form the process filename and args for
    // the model build Perl script.
    string plcmd = psethome();
    plcmd.append("/bin/disperl");
    string mbcmd = psethome();
    mbcmd.append("/mb/pbmb_driver.pl");
    static char *argv[] = { 
	NULL, // pbmb_driver.pl
	NULL, // -dsp, -derived, or -project
	NULL, // DSP
	"-lpservice",
	NULL, // name of service to log presentation
	"-admindir",
	NULL, // pathname for admindir
	NULL, // -qaTaskFile 
	NULL, // name of task file for QAR
	NULL };
    argv[0] = (char *)mbcmd.c_str();
    if (projectKind == pk_from_extension_dflt_dsp) {
	argv[1] = "-dsp";
    }
    else if (projectKind == pk_clearmake) {
	argv[1] = "-derived";
    }
    else if (projectKind == pk_defer) {
	argv[1] = "-project";
    }
    else {
	argv[1] = "-dsp";
    }
    argv[2] = (char *)dspPath.c_str();
    argv[4] = (char *)lpservice();
    argv[6] = (char *)outputPath.c_str();
    if (qaTaskFile) {
	argv[7] = "-qaTaskFile";
	argv[8] = (char *)qaTaskFile->c_str();
    }

    // Start the thread which waits for the model build to finish.
    terminated = false;
    Procedure *builder_procedure = new MBThread(mb_pid, plcmd.c_str(), argv, send2gui);
    builder_thread = new Thread("wait on mb process");
    builder_thread->start(*builder_procedure);
}

// ===========================================================================
//
void MBDriver::start_model_build_process(const string &cmd, vector<string> &argvs,
					 bool send2gui(Message &msg))
{
    char **argv = new char *[argvs.size() + 3];
    vector<string>::iterator iter;
    int n = 0;
    for (iter = argvs.begin(); iter != argvs.end(); ++iter) {
	argv[n++] = (char *)(*iter).c_str();
    }
    argv[n++] = "-lpservice";
    argv[n++] = (char *)lpservice();
    argv[n++] = NULL;

    // Start the thread which waits for the model build to finish.
    terminated = false;
    Procedure *builder_procedure = new MBThread(mb_pid, cmd.c_str(), argv, send2gui);
    builder_thread = new Thread("wait on mb process");
    builder_thread->start(*builder_procedure);

    delete [] argv;
}

// ===========================================================================
//
void MBDriver::shut_down_model_build_process()
{
    // Ensure the model build is done.
    MBDriver::pid_t local_mb_pid;
    mb_guard.lock();
    local_mb_pid = mb_pid;
    mb_guard.unlock();
    if (builder_thread != NULL && local_mb_pid != null_pid) {
	terminate_process(local_mb_pid, 1); // used to check for builder_thread->isAlive().
#if 0
	// It's not clear that this helps, and it may cause
	// unanticipated problems.

	// Wait until the thread tracking the model build shutdown
	// has completed its job.
	if (builder_thread != 0) {
	    Exclusion track_thread(*builder_thread, "wait for model build");
	}
#endif
    }
}

// ===========================================================================
//
void MBDriver::cancel_model_build_process()
{
    MBDriver::pid_t local_mb_pid;
    mb_guard.lock();
    local_mb_pid = mb_pid;
    mb_pid = null_pid;
    terminated = true;
    mb_guard.unlock();
    terminate_process(local_mb_pid, 1);
}
