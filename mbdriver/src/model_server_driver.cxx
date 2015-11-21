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
#include "model_server_driver.h"
#include "threads.h"
#include "transport.h"
#include "mbuildmsg.h"
#include "sys/types.h"
#include "model_server_utils.h"
#include "service.h"
#include "mbdriver_util.h"
#include "util.h"
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

static MBDriver::pid_t model_server_pid = null_pid;
static MBDriver::pid_t qa_engine_pid = null_pid;

//=============================================================================
// 
static Monitor mon_kill_model_server;
static bool kill_model_server = false;
void MBDriver::terminate_model_server()
{
    Exclusion sync(mon_kill_model_server, "set kill_model_server");
    kill_model_server = true;
}

static bool is_kill_model_server()
{
    Exclusion sync(mon_kill_model_server, "check kill_model_server");
    return (kill_model_server);
}

//=============================================================================
static string get_model_server_shutdown_time(string const *tstr)
{
    string time_str = "";
    if (tstr) {
	long seconds = atol(tstr->c_str());
	if (seconds > 0) {
	    time_str = *tstr;
	}
	else {
	    time_str = "3600"; //default 1 hour
	}
    }
    return time_str;
}


// ###########################################################################
//============================================================================
//
class PmodServerThread : public Procedure
{
public:
    PmodServerThread(MBDriver::processIdType ps_pid, string sname, string *mdir, string logfile, 
		     Connection<Message>& conn)
	:   gui_connection(conn),
	    log_filename(logfile),
	    model_dir(mdir != NULL ? new string(*mdir) : NULL),
	    modelServerPid(ps_pid), 
	    service_name(sname){}
    ~PmodServerThread() { delete model_dir; }

    virtual void run();
private:
    bool start_qa_engine(string&);
    MBDriver::pid_t start_code_rover(string&);

    Connection<Message> &gui_connection;
    string log_filename;
    const string* model_dir;
    MBDriver::processIdType modelServerPid;
    string service_name;
};

void
PmodServerThread::run()
{
	MBDriver::pid_t codeRoverPid = null_pid;
	string error_message;

	string modelServiceName = get_internal_service_name(service_name);
	ServiceName model_service(modelServiceName.c_str());
	//poll nameserver every second to see if the model server has 
	//registered itself. While waiting check if user has pressed
	//cancel button or if model server exited for some reason.
	int port;
	int ipaddr;
	bool exit_flag = false;
	bool srvr_died = false;

	long srvr_die_value;
	while (!srvr_died && !exit_flag && !model_service.getPortAndAddr(port, ipaddr)) {
		//check if user has selected "cancel"
		exit_flag = ::is_kill_model_server();

		//check if model server is still alive
		srvr_died = !is_process_alive(model_server_pid, &srvr_die_value);
		if (!srvr_died && !exit_flag) {
		    sleep_in_seconds(1);
		}
	}
	if (srvr_died) { //model server died
#ifdef ISO_CPP_HEADERS
	    ostringstream strs;
#else
	    ostrstream strs;
#endif
	    strs << "The model server exited with a return value of " << srvr_die_value << ".\n";
	    strs << "Please view the log file: " << log_filename.c_str() << ".\n";
#ifndef ISO_CPP_HEADERS
	    strs << ends;
#endif
	    error_message = strs.str();
	}
	else if (exit_flag) //cancel button has been pressed, terminate model server
	    terminate_process(model_server_pid);
	else {
	    if (trace_sends)
		dbgprt("Model server registered with name server");
	    bool qa_ok = true;
	    if (hasCaliperLicense()) {
	        qa_ok = start_qa_engine(error_message);
		if (!qa_ok) {
		    //QAEngine did not start, kill model server.
		    terminate_process(model_server_pid);
		}
	    }
	    if (qa_ok) {
		//model server has registered itself, start code rover browser now
		codeRoverPid = start_code_rover(error_message);
	    }
	}
	//send status of model server/code rover startup to the GUI.
	Message *msg = 0;
	if (codeRoverPid != null_pid) {
	    msg = new Message(DOMString(codeRoverStartedXMLStr));
	} else {
	    msg = new Message(DOMString(codeRoverFailedXMLStr));
	    //In case of failure "try" to send reason for failure
	    if (!error_message.empty()) {
		DOMString emsg;
		trivialToDOMString(emsg, error_message.c_str());
		msg->SetText(emsg);
	    }
	}
	if (msg) {
	    gui_connection.send(*msg);
	    delete msg;
	}
	delete this;
}

bool
PmodServerThread::start_qa_engine(string& error_message)
{
    bool ok = true;
  
    char model_server_pid_str[256];
    sprintf(model_server_pid_str, "%lu", modelServerPid);

    string qa_service_name = "QA:";
    qa_service_name += service_name;
    ServiceName qa_service(qa_service_name.c_str());

    int port,ipaddr;
    if (!qa_service.getPortAndAddr(port, ipaddr)) { //If QAEngine is not running
	char *p = getenv("PSETHOME");       //start it now.
	string qa_engine_cmd;
	if (p) qa_engine_cmd = p;
	else qa_engine_cmd = "";
	qa_engine_cmd.append("/bin/QAEngine");
    
	// qa_root = $ADMINDIR/QA 
	string qa_root;
	if (!model_dir->empty()) {
	    qa_root = model_dir->c_str();
	}
	else {
	    qa_root = getenv("ADMINDIR");
	}
	qa_root += "/QA";

	// qa_projects_dir = $PSETHOME/lib/policy
	string qa_projects_dir = psethome();
	qa_projects_dir += "/lib/policy";

	char *args[] = {
	        NULL,	    //QAEngine service name
		NULL,	    //QA build root directory
		NULL,	    //QA projects directory
		NULL,       //model_server PID
		NULL };
	args[0] = (char *)qa_service_name.c_str();
	args[1] = (char *)qa_root.c_str();
	args[2] = (char *)qa_projects_dir.c_str();
	args[3] = (char *)model_server_pid_str;

	if (trace_sends) {
	    string str="starting QAEngine with command: ";
	    str.append(qa_engine_cmd.c_str());
	    for (int i=0; i<4;i++) {
		if (args[i]) {
		    str.append(" "); 
		    str.append(args[i]);
		}
	    }
	    dbgprt(str.c_str());
	}

	qa_engine_pid = start_process(qa_engine_cmd.c_str(), args);
	if (qa_engine_pid != null_pid) {
	    //poll nameserver every second to see if the QAEngine has 
	    //registered itself. While waiting check if user has pressed
	    //cancel button or if QAEngine exited.
	    bool exit_flag = false, qaengine_died = false;
	    long qa_die_value;
	    while (!qaengine_died && !exit_flag && !qa_service.getPortAndAddr(port, ipaddr)) {
		//check if user has selected "cancel"
		exit_flag = ::is_kill_model_server();

		//check if QAEngine is still alive
		qaengine_died = !is_process_alive(qa_engine_pid, &qa_die_value);
		if (!qaengine_died && !exit_flag) {
		    sleep_in_seconds(1);
		}
	    }
	    if (qaengine_died) {
#ifdef ISO_CPP_HEADERS
		ostringstream strs;
#else
		ostrstream strs;
#endif
		strs << "QAEngine exited with a return value of " << qa_die_value << ".\n";
#ifndef ISO_CPP_HEADERS
	        strs << ends;
#endif
		error_message = strs.str();
		ok = false;
	    }
	    else if (exit_flag) { //cancel button has been pressed, terminate QAEngine
		terminate_process(qa_engine_pid);
		ok = false;
	    }
	    else {
		//everything is fine, QAEngine has registered with name server
		if (trace_sends)
		    dbgprt("QAEngine registered with name server.");
	    }
	}
	else {
	    error_message = "Failed to start QAEngine process.";
	    ok = false;
	}
    }
    else {
	if (trace_sends)
	    dbgprt("Using existing QAEngine.");
    }
    return ok;
}

MBDriver::pid_t
PmodServerThread::start_code_rover(string& error_message)
{
    MBDriver::pid_t codeRover_pid = null_pid;

    //prepare the configuration file for code rover
    string cr_filename;
    if (!model_dir->empty()) {
	cr_filename = model_dir->c_str();
    }
    else {
	cr_filename = getenv("ADMINDIR");
    }
    cr_filename.append("/");
    cr_filename.append(service_name.c_str());
    cr_filename.append(".cr");
    FILE* cr_file = fopen(cr_filename.c_str(), "w");
    if (cr_file == NULL) {
#ifdef ISO_CPP_HEADERS
	ostringstream str;
#else
	ostrstream str;
#endif
	str << "Could not open file " << cr_filename.c_str() << " to write information for Code Rover Browser.";
#ifndef ISO_CPP_HEADERS
	str << ends;
#endif
	error_message = str.str();
    } else {
	//add Host value, IP address for "localhost"
	fprintf(cr_file,"Host : 127.0.0.1\n");
	//add Model name(model server service name)
	fprintf(cr_file, "Model : %s\n", service_name.c_str());
	//set connection type to LAN
	fprintf(cr_file, "Connection : -lan\n");
	fclose(cr_file);

	char *p = getenv("PSETHOME");
	string code_rover_cmd;
	if (p)
	    code_rover_cmd = p;
	else
	    code_rover_cmd = "";
	code_rover_cmd.append("/bin/DevXLauncher");
	char *args[] = {
	    NULL,
	    NULL };
	args[0] = (char *)cr_filename.c_str();

	//print command for debugging purposes
	string str="starting code rover browser with command: ";
	str.append(code_rover_cmd.c_str());
	str.append(" ");
	str.append(args[0]);
	if (trace_sends)
	    dbgprt(str.c_str());

	codeRover_pid = start_process(code_rover_cmd.c_str(), args);
	if (codeRover_pid == null_pid) {
#ifdef ISO_CPP_HEADERS
	    ostringstream str;
#else
	    ostrstream str;
#endif
	    str << "Code Rover Browser process failed with a return value of ";
#ifdef _WIN32
	    DWORD val = GetLastError();
	    str << val;
#endif
#ifndef ISO_CPP_HEADERS
	    str << ends;
#endif
	    error_message = str.str();
	}
    }

    return codeRover_pid;
}

// ===========================================================================
//
void MBDriver::start_model_server_process(string dsp_path,
                                         string output_path,
					 Connection<Message> &gui_connection,
					 string const *server_time)
{
    string modelservice = get_service_name(&dsp_path, &output_path);
    string service_string = get_internal_service_name(modelservice);

    string model_server_log = output_path.c_str();
    model_server_log.append("/model_server.log");

    string shutdown_time = get_model_server_shutdown_time(server_time);

    processIdType ms_pid = 0;

    //check if model server is running otherwise start a new one
    bool ok = true;
    string error_msg;
    int port;
    int paddr;
    ServiceName sn(service_string.c_str());
    if (!sn.getPortAndAddr(port, paddr)) {
	//model server is not running 
	//check if model is built before starting model server... 
	if (validate_model_dir(&output_path)) {
	    if( check_lm_license_file(error_msg) ) {
		string pset_home = psethome();
		string model_server_cmd = pset_home;
		model_server_cmd.append("/bin/model_server.exe");
		string prefs_arg = output_path.c_str();
		prefs_arg.append("/prefs/build.prefs");
		string pdfFile = output_path.c_str();
		pdfFile.append("/pdf/build.pdf");
				
		static char *args[] = {
		    "-prefs",		    // 0
		    NULL,		    // 1
		    "-pdf",		    // 2
		    NULL,		    // 3
		    "-service",		    // 4
		    NULL,		    // 5
		    "-log_file",	    // 6
		    NULL,		    // 7
		    "-auto_shutdown",	    // 8
		    NULL,		    // 9
		    NULL };
			
		args[1] = (char *)prefs_arg.c_str();
		args[3] = (char *)pdfFile.c_str();
		args[5] = (char *)modelservice.c_str();
		args[7] = (char *)model_server_log.c_str();
		if (!shutdown_time.empty()) {
		    args[9] = (char *)shutdown_time.c_str();
		}
		else {
		    //If shutdown_time is empty do not send -auto_shutdown command
		    //line option to the server. This will keep server running forever. 
		    args[8] = NULL;
		}
		set_environment_for_model_server(output_path.c_str(), pset_home.c_str());
		if (trace_sends) {
		    string str="starting model server with command: ";
		    str.append(model_server_cmd.c_str());
		    for (int i=0; i<10;i++) {
			if (args[i]) {
			    str.append(" "); 
			    str.append(args[i]);
			}
		    }
		    dbgprt(str.c_str());
		}

		model_server_pid = start_process_get_pid(model_server_cmd.c_str(), 
		                                         args, 0, ms_pid);
		if (model_server_pid == null_pid) {
		    error_msg = "Failed to start model server process.";
		    ok = false;
		}

	    } else {
		ok = false;
	    }
	} else {
	    error_msg = "Information model is not built.\n";
	    error_msg.append("Failed to find model in directory: ");
	    error_msg.append(output_path.c_str());
	    ok = false;
	}
    }
    if (ok) {
	Procedure *modelserver_thread_procedure = 
	    new PmodServerThread(ms_pid, modelservice, &output_path, 
				 model_server_log, gui_connection);
	Thread modelserver_thread("model server thread");
	modelserver_thread.start(*modelserver_thread_procedure);
    } else {
	Message msg(static_cast<DOMString>(codeRoverFailedXMLStr));
	DOMString emsg;
	trivialToDOMString(emsg, error_msg.c_str());
	msg.SetText(emsg); 
	gui_connection.send(msg);
    }
}

// ===========================================================================
//
void MBDriver::shut_down_model_server (const string *dsp_path, const string *output_path)
{
    //Before starting a new model build terminate the model server.
    if (model_server_pid != null_pid) {
	terminate_process(model_server_pid);
	if (qa_engine_pid != null_pid) {
	    terminate_process(qa_engine_pid);
	}
    }
    else
	stop_model_server(dsp_path, output_path);
}
