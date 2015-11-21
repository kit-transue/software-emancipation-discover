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
#pragma warning(disable:4786)

#include "startprocess.h"

#ifdef TRANSPORT_SERVER
#ifndef MULTITHREADED
#define MULTITHREADED 1
#endif
#endif

#include <string>
#include <stdlib.h>
#ifdef MULTITHREADED
#include "threads.h"
using namespace SAThreads;
#endif
#include <set>
#ifndef DIRECT_MSG
#include "msg.h"
#else
// Don't want to link serverspy with msg code.
#include <stdio.h>
#endif

#ifndef _WIN32
   #include <signal.h>
   #include <unistd.h>
   #include <sys/wait.h>
   #include <sys/errno.h>
   #include <errno.h>
   #include <string.h>
   #include <stdio.h>
#endif

#ifdef ISO_CPP_HEADERS
   #include <iostream>
   #include <sstream>
   #include <fstream>
#else
   #include <iostream.h>
   #include <strstream.h>
   #include <fstream.h>
#endif

using namespace MBDriver;
namespace std {};
using namespace std;

static bool log_windows_enabled = false;

//========================================================================
void MBDriver::enable_log_windows()
{
    log_windows_enabled = true;
}

bool MBDriver::is_log_windows_enabled()
{
    return log_windows_enabled;
}

#ifdef _WIN32
// ===========================================================================
// NT implementation
//	
static HANDLE start_cmd(const char *executable, char* cmd, int detach_flag,
                        HANDLE *hOutputRead, processIdType& process_id) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;     
    HANDLE hOutputWrite;

    if (hOutputRead != NULL) {
        HANDLE hOutputReadTmp;
	SECURITY_ATTRIBUTES sa;

	// Set up the security attributes struct.
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	// Create the child output pipe.
	CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 0);

	// Create output read handle. Set the Properties to FALSE.
	// Otherwise, the child inherits a handle it's not going to close.
	// Close inheritable copy of the handle.
	DuplicateHandle(GetCurrentProcess(), hOutputReadTmp,
			     GetCurrentProcess(),
			     hOutputRead, // Address of new handle.
			     0, FALSE, // Make it uninheritable.
			     DUPLICATE_SAME_ACCESS);
	CloseHandle(hOutputReadTmp);
    }

    memset(&si,0,sizeof(STARTUPINFO));
    si.wShowWindow=SW_HIDE;
    si.cb=sizeof(STARTUPINFO);
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = (hOutputRead == NULL ? GetStdHandle(STD_OUTPUT_HANDLE)
                                    : hOutputWrite);
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags    = STARTF_USESTDHANDLES;

    int detach_process = (detach_flag == 1) ? DETACHED_PROCESS : 0;
    if(::CreateProcess(executable,cmd,NULL,NULL,hOutputRead != NULL,
		       log_windows_enabled ? CREATE_NEW_CONSOLE : (detach_process|CREATE_NO_WINDOW),
		       NULL,NULL,&si,&pi
		      )==FALSE) {
	process_id = 0;
	return NULL;
    }
    else {
	if (hOutputRead != NULL) {
	    // Close unnecessary handle.
	    CloseHandle(pi.hThread);
	    CloseHandle(hOutputWrite);
	}
	process_id = pi.dwProcessId;
	return pi.hProcess;
    }
}

//========================================================================
// Event used to forcibly kill the spawned Process Group. This event object
// is set when a process group is to be terminated in terminate_processgroup.
// The spawn_process executable, which waits for this event object to be 
// set, generates a Ctrl-Break-Event to all the processes in the process 
// group which causes all processes in the group to terminate.
// All this is required since NT provides no easy way of terminating 
// detached spawned processes.
static HANDLE g_heventTerminateProcessGroup = NULL;

static HANDLE startProcessGroup(char *cmd) {
    // Create an inheritable event handle to use as our IPC mechanism
    // to terminate the processes in the process group
    if (g_heventTerminateProcessGroup == NULL)
        g_heventTerminateProcessGroup = CreateEvent(NULL, TRUE, FALSE, NULL);
    SetHandleInformation(g_heventTerminateProcessGroup, 
                         HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    ResetEvent(g_heventTerminateProcessGroup); 

    char cmdline[1024];
    sprintf(cmdline, "spawnprocess.exe %d ", g_heventTerminateProcessGroup);
    string szCmdLine = cmdline;
    szCmdLine.append(cmd);


    STARTUPINFO si = { sizeof(si) };
    si.dwFlags     = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;  // Processes in the Process Group are hidden
 
    PROCESS_INFORMATION pi;
    // Spawn the process (make sure it inherits our event handle)
    BOOL f = CreateProcess(NULL, (char *)szCmdLine.c_str(), NULL, NULL, TRUE, 
                           CREATE_NEW_PROCESS_GROUP|
			   (log_windows_enabled ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW),
			   NULL, NULL, &si, &pi);
    if (!f)
#ifndef DIRECT_MSG
        msg("Failed to create process group for command: $1", error_sev) << szCmdLine << eoarg << eom;
#else
        fprintf(stderr, "Failed to create process group for command: %s\n", szCmdLine.c_str());
#endif
    return pi.hProcess;
}

// Set the event object which will kill the process group
static void terminate_processgroup() {
    SetEvent(g_heventTerminateProcessGroup);
}

// ===========================================================================
// Quote one argument so it can appear in an NT command and be passed
// along to the C program.
//
static string quote(const string &str)
{
    string result;
    result.append("\"");
    result.append(str);
    result.append("\"");
    return result;
}

// ===========================================================================
// Translate the arg list into a single-string command.
//
// detach_flag argument is used while creating the new process. It is a 
// default argument and its value is set to 0. 
// If detach_flag 
//	= 1, make the new process a detached one 
//	= 2, create a new process group
//  = otherwise make the new process child of the current process
//
static MBDriver::pid_t start(const char *pathp, const char *const argv[],
                             int detach_flag, HANDLE *hOutputRead,
			     processIdType& process_id)
{
    // Prepare command string in a writable buffer.
    list<string> arg_list;
    arg_list.push_back(pathp);
    for (int i = 0;; i += 1) {
	const char *arg = argv[i];
	if (arg == NULL) {
	    break;
	}
	arg_list.push_back(arg);
    }
    string cmd = prepare_command_arguments(arg_list);
    char *cmd_buf = new char[cmd.length() + 1];
    strcpy(cmd_buf, cmd.c_str());

    // Prepare path of executable.
    string path = pathp;
    int length = path.size();
    const char *tail = pathp + (length - 4);
    if (length >= 4 && (strcmp(tail, ".exe") == 0 || strcmp(tail, ".EXE") == 0)) {
    }
    else {
	// append .exe
	path.append(".exe");
    }
    
    MBDriver::pid_t result;
    if (detach_flag == 2)
	//name of executable occurs first in cmd_buf
        result = startProcessGroup(cmd_buf);
    else
        result = start_cmd(path.c_str(), cmd_buf, detach_flag, hOutputRead, process_id);
    delete [] cmd_buf;
    return result;
}

#else
// ==========================================================================
// Adds path to the array of arguments to be passed to execv.
// Caller is to delete returned array.
//
static const char **fix_args(const char *path, const char *const argv[]) {
    int num_args = 0;
    while ( argv[num_args] ) {
      num_args++;
    }
    const char **new_args = new const char *[num_args+2];
    new_args[0] = path;            // first argument of array is path 
    for (int i=0; i<num_args; i++) {
      if (argv[i]) {
	new_args[i+1] = argv[i];
      }
    }
    //terminate argument array with null pointer
    new_args[num_args+1] = NULL;
    return new_args;
}

// ===========================================================================
// UNIX implementation
// Returns null_pid (0) if starting fails.
//
static MBDriver::pid_t start(const char *path, const char *const argv[],
                             int detach_flag, FILE **opipe,
			     processIdType& process_id) {
    // Capture the standard output via a pipe.
    int fd[2];
    if (opipe != NULL && pipe(fd)) {
#ifndef DIRECT_MSG
	msg("cannot create pipe", error_sev) << eom;
#else
	fprintf(stderr, "cannot create pipe\n");
#endif
	return 1;
    }
    const char **new_args = fix_args(path, argv);
#if 0
    printf ("fork with path \"%s\" and args", path);
    for (const char **arg = argv; *arg != NULL; arg += 1) {
	printf(" \"%s\"", *arg);
    }
    printf("\n");
#endif
    const MBDriver::pid_t pid
#if !defined(sun5)
	= fork();
#else // sun5
	= fork1();
#endif
    if (pid < 0) {
#ifndef DIRECT_MSG
	msg("cannot fork") << eom;
#else
	fprintf(stderr, "cannot fork\n");
#endif
	if (opipe != NULL) {
	    *opipe = NULL;
	    close(fd[0]);
	    close(fd[1]);
	}
	return 0;
    }
    if (pid == 0) {		// CHILD
	setpgid(0,0);
	if (opipe != NULL) {
	    // Reading stdout from child
	    close(fd[0]);
	    if (dup2(fd[1], 1) < 0) {
#ifndef DIRECT_MSG
		msg("cannot setup stdout for child") << eom;
#else
		fprintf(stderr, "cannot setup stdout for child\n");
#endif
		exit(1);
	    }
	    close(0);
	    // open("/dev/null", O_RDONLY);  ? ? ?
	}
	int r = execv(path, (char **)new_args);
	int en = errno;
	perror("exec failure");
#ifndef DIRECT_MSG
	msg("exec $1 failed $2, errno $3") << path << eoarg << r << eoarg << en << eoarg << eom;
#else
	fprintf(stderr, "exec %s failed %d, errno %d\n", path, r, en);
#endif
	exit(1);
    }
    else {			// PARENT
	if (opipe != NULL) {
	    *opipe = fdopen(fd[0], "r");
	    close(fd[1]);
	}
    }
    delete [] new_args;
    process_id = pid;
    return pid;
}
#endif

// ===========================================================================
MBDriver::pid_t MBDriver::start_process(const char *path,
                                        const char *const argv[],
                                        int detach_flag)
{
    processIdType process_id = 0;
    return start(path, argv, detach_flag, NULL, process_id);
}


// ===========================================================================
// Same as the above function except this function also gets process ID.
// On Unix these two functions are the same since the return value is also the
// process ID. On NT the return value is process handle and not process ID. Thus
// this function will be used in places where process ID is needed.
// 
MBDriver::pid_t MBDriver::start_process_get_pid(const char *path,
                                                const char *const argv[],
                                                int detach_flag,
						processIdType& process_id)
{
    return start(path, argv, detach_flag, NULL, process_id);
}

// ===========================================================================
#ifdef MULTITHREADED
static set<MBDriver::pid_t> *awaited = NULL;
static Mutex guard_awaited;
static Monitor guard_process;
static MBDriver::pid_t ready_pid = null_pid;
static int ready_status;
#endif

// ===========================================================================
// Returns the status from the awaited process.  Success is 0.
//
int MBDriver::wait_process(MBDriver::pid_t pid)
{
    int status = 0;
    if (pid == null_pid) {
	return 0;
    }
#ifdef _WIN32
    DWORD dstatus = 0;
    WaitForSingleObject(pid, INFINITE);
    GetExitCodeProcess(pid, &dstatus);
    status = dstatus;
#else // Unix, not NT
#ifndef MULTITHREADED
    MBDriver::pid_t ready_pid = null_pid;
    while (ready_pid != pid) {
	ready_pid = ::wait(&status);
	if (status == EINTR) {  // just an interrupt; no process done
	    ready_pid = null_pid;
	}
    }
#else // MULTITHREADED
    // !!! This is not tested. !!!
    // Since Unix 'wait' returns when _any_ child process terminates,
    // it is necessary to coordinate all waiters.  It is an error if
    // more than one thread waits on the same child process.

    // Only one thread will call ::wait.  The others wait on the monitor.
    {
	Exclusion s(guard_awaited);
	if (awaited == NULL) {
	    awaited = new set<MBDriver::pid_t>;
	}
	if (awaited->find(pid) != awaited->end()) {
	    // internal error: 2 threads await 1 child.
	    return 1;
	}
	awaited->insert(pid);
    }
    {
	Exclusion s(guard_process);
	while (ready_pid != pid) {
	    if (ready_pid == null_pid) {
		MBDriver::pid_t done_pid = ::wait(&status);
		if (status == EINTR) {  // just an interrupt; no process done
		    done_pid = null_pid;
		}
		{
		    Exclusion s(guard_awaited);
		    if (awaited->find(done_pid) != awaited->end()) {
			// a process we were waiting for
			ready_pid = done_pid;
			ready_status = status;
		    }
		}
		if (ready_pid != pid && ready_pid != null_pid) {
		    guard_process.notify();
		}
	    }
	    else {
		guard_process.wait();
	    }
	}
	status = ready_status;
	ready_pid = null_pid;
	guard_process.notify();
    }
    {
	Exclusion s(guard_awaited);
	awaited->erase(pid);
    }
#endif // MULTITHREADED
#endif // Unix, not NT
    return status;
}

// ===========================================================================
//
MBDriver::pid_t MBDriver::get_pid()
{
#ifdef _WIN32
    return GetCurrentProcess();
#else // Unix
    return ::getpid();
#endif // Unix vs. NT
}

// ===========================================================================
//
bool MBDriver::is_process_alive(pid_t pid, long *status)
{
    bool alive = true;
#if _WIN32
    unsigned long srvr_die_value;
    GetExitCodeProcess(pid, &srvr_die_value);
    if (srvr_die_value != STILL_ACTIVE) {
	alive = false;
    }
    if (status != NULL) {
        *status = (long)srvr_die_value;
    }
#else
    int wait_status;
    if (waitpid(pid, &wait_status, WNOHANG) > 0) {
	alive = false;
    }
    if (status != NULL) {
        *status = (long)wait_status;
    }
#endif // _WIN32
    return alive;
}

// ===========================================================================
// Returns the status of the started process (0 indicating success)
// or 1 if an error occurred while starting the process.  If 'output'
// is not null, standard output of the process is captured into that string.
//
int MBDriver::start_process_and_wait(const char *path, const char *const argv[], string *output)
{
    processIdType process_id;
#if _WIN32
    HANDLE hOutputRead = NULL;
    MBDriver::pid_t pid = start(path, argv, 0,
                                output != NULL ? &hOutputRead : NULL,
				process_id);

    if (output != NULL) {
#ifdef ISO_CPP_HEADERS
	ostringstream ostrm;
#else
	ostrstream ostrm;
#endif
	if (hOutputRead != NULL) {
	    // Monitor pipe for input from the child.
	    // Exit when the child exits or pipe breaks.
	    CHAR lpBuffer[256];
	    DWORD nBytesRead;
	    while(TRUE)
	    {
		if (!ReadFile(hOutputRead, lpBuffer, sizeof(lpBuffer), &nBytesRead, NULL)
		    || !nBytesRead) {
		   if (GetLastError() == ERROR_BROKEN_PIPE)
		      break; // pipe done - normal exit path.
		}

		// Copy characters read.
		ostrm.write(lpBuffer, nBytesRead);
	    }
	    // Redirection is complete
	    CloseHandle(hOutputRead);
	}
#ifndef ISO_CPP_HEADERS
	ostrm << ends;
#endif
	*output = ostrm.str();
    }
#else   // Unix
    FILE *outputRead = NULL;
    const MBDriver::pid_t pid = start(path, argv, 0,
				      output != NULL ? &outputRead : NULL,
				      process_id);
    if (output != NULL) {
#ifdef ISO_CPP_HEADERS
	ostringstream ostrm;
#else
	ostrstream ostrm;
#endif
	if (outputRead != NULL) {
	    // Monitor pipe for input from the child.
	    // Exit when the child exits or pipe breaks.
	    char buffer[256];
#if 0
	    long nBytesRead;
            int odescr = fileno(outputRead);
#else
	    size_t nBytesRead;
#endif
	    while (true) {
#if 0
		nBytesRead = read(odescr, buffer, sizeof(buffer));
		if (nBytesRead < 0) {
		    if (errno == EAGAIN || errno == EINTR) {
			// OK -- continue
			nBytesRead = 0;
		    }
		    else {
			break;
		    }
		}
		else if (nBytesRead == 0) {
		    break; // pipe done - normal exit path.
		}
#else
		nBytesRead = fread(buffer, 1, sizeof(buffer), outputRead);
		if (nBytesRead == 0) {
		    if (feof(outputRead)) {
		        break; // pipe done - normal exit path.
		    }
		    if (ferror(outputRead)) {
    	    	    	break;
		    }
		}
#endif

		// Copy characters read.
		ostrm.write(buffer, nBytesRead);
	    }
	    // Redirection is complete
	    fclose(outputRead);
	}
#ifndef ISO_CPP_HEADERS
	ostrm << ends;
#endif
	*output = ostrm.str();
    }
#endif  // NT/Unix
    return wait_process(pid);
}

// ===========================================================================
// if processgroup is set the whole process group is terminated 
void MBDriver::terminate_process(MBDriver::pid_t pid, bool processgroup) {
    if (pid != null_pid) {
#ifdef _WIN32
        if (processgroup)
            terminate_processgroup();
	else 
	    TerminateProcess(pid, 0);
	CloseHandle(pid);
#else
	if (processgroup)
	    kill(-pid, SIGTERM);
	else
	    kill(pid, SIGTERM);
#endif
    }
}

// ===========================================================================
// To pass command arguments, shells and 'system' functions use a single text
// strings rather than lists of arguments.  Distinct contexts use different
// conventions for representing lists.  We sometimes use Tcl conventions
// internally.
//
#ifndef _WIN32
// ===========================================================================
// On Unix, we hope that supporting the Bourne shell escaping conventions
// will be adequate.  Actually, this is called from driver/mb during the
// analysis pass of model build for the sake of dismb, so it's destined
// for Perl's 'system' call rather than the Bourne shell.
//
// The arguments are concatenated, with spaces inserted as separators.
// Any argument containing special characters ($'\" >*|<?[]&`(){}#=) is placed
// in quotes.
// Escaping with backslash doesn't work inside single quotes unless the
// escaped character is $ or \, because backslash means to continue at the
// following input line.  We use double quotes for consistency with Windows
// and with what has been done in the past. Within a quoted argument,
// the characters backslash (\), double-quote ("), dollar ($), and
// backquote (`) are escaped with a backslash.
// 
string MBDriver::prepare_command_arguments(const list<string> &args)
{
    string result;
    int arg_count = 0;

    list<string>::const_iterator iter;
    for (iter = args.begin(); iter != args.end(); ++iter) {
	string arg = *iter;
	const char *p = arg.c_str();

	if (arg_count > 0) {
	    result.append(1, ' ');
	}
	if (strpbrk(p, "$'\\\" \t>*|<?[]&`(){}#=") != NULL) {
	    // Argument contains a special character; quote it.
	    result.append(1, '"');

	    for (; *p != '\0'; p += 1) {
		char ch = *p;
		if (ch == '"' || ch == '$' || ch == '`' || ch == '\\') {
		    result.append(1, '\\');
		}
		result.append(1, ch);
	    }

	    // Close the quote.
	    result.append(1, '"');
	}
	else {
	    result.append(arg);
	}
	arg_count += 1;
    }
    return result;
}

// ===========================================================================
// To manage a command line on Unix, it is necessary to transform a
// single string into a list of strings.  The arguments found in cmdline
// are appended to the given cmdargs list.
//
// Backslashes, single-quotes, and double-quotes are respected.
// 
void MBDriver::determine_command_arguments(const string &cmdline,
					   list<string> &cmdargs) {
    const char *p = cmdline.c_str();
    const char *start = NULL;
    string arg;
    char in_quote = '\0';
    for (;;) {
	// p points to the current command line character being examined.
	// start indicates the beginning of the string which needs to be
	// copied to the argument.
        const char ch = *p;
        if (start != NULL && (ch == '\0' || (ch == ' ' && in_quote == '\0'))) {
	    arg.append(start, p - start);
	    cmdargs.push_back(arg);
	    start = NULL;
	}
	if (ch == '\0') {
	    break;
	}
	if (start == NULL && !(ch == ' ' || ch == '\t')) {
	    start = p;
	    arg = "";
	}
	if (start == NULL) {
	    // Wait for a non-whitespace character.
	}
	else if (in_quote == '\'') {
	    if (ch == '\\' && p[1] == '\\') {
		arg.append(start, p - start);
		start = p + 1;
		p = start;
	    }
	    else if (ch == '\'') {
		in_quote = '\0';
		arg.append(start, p - start);
		start = p + 1;
	    }
	    else {
		// Accept ch.
	    }
	}
	else if (in_quote == '"') {
	    if (ch == '\\' && (p[1] != '\'' && p[1] != '\0')) {
		arg.append(start, p - start);
		start = p + 1;
		p = start;
	    }
	    else if (ch == '"') {
		in_quote = '\0';
		arg.append(start, p - start);
		start = p + 1;
	    }
	    else {
		// Accept ch.
	    }
	}
	else if (ch == '\\' && p[1] != '\0') {
	    arg.append(start, p - start);
	    start = p + 1;
	    p = start;
	}
	else if (ch == '"') {
	    in_quote = ch;
	    arg.append(start, p - start);
	    start = p + 1;
	}
	else if (ch == '\'') {
	    in_quote = ch;
	    arg.append(start, p - start);
	    start = p + 1;
	}
	p += 1;
    }
}

#else // defined(_WIN32)
// ===========================================================================
// To pass command arguments on Windows, it is necessary to transform a
// list of strings into a single string.
//
// The arguments are concatenated, with spaces inserted as separators.
// Arguments that contains spaces are double-quoted.
// Quotes are escaped using a backslash.
// Note that unlike C strings, a single backslash represents itself.
// If this procedure were applied to an argument which contained a space
// and whose final character was a backslash, the transformed result
// contains a backslash-quote combination, which is interpreted by
// Windows as an escaped double-quote.  Therefore we advance the quote
// to follow the last non-backslash character.
// 
string MBDriver::prepare_command_arguments(const list<string> &args)
{
    string result;
    int arg_count = 0;

    list<string>::const_iterator iter;
    for (iter = args.begin(); iter != args.end(); ++iter) {
	string arg = *iter;

        // Escape the double-quotes.
	string::size_type n = 0;
	for (;;) {
	    string::size_type where = arg.find("\"", n);
	    if (where == string::npos) {
		break;
	    }
	    arg.replace(where, 1, "\\\"");
	    n = where + 2;
	}

        // Quote the spaces.
	// Quotes are used for %'s not because they are strictly required,
	// but because parsercmd has an unusual need for this.
	if (arg.find(" ") != string::npos || arg.length() == 0
		|| arg.find("%") != string::npos) {
	    string::size_type end_quote = arg.find_last_not_of('\\');
	    arg.insert(end_quote == string::npos ? 0 : end_quote + 1, 1, '"');
	    arg.insert(string::size_type(0), 1, '"');
	}

	if (arg_count > 0) {
	    result.append(1, ' ');
	}
	result.append(arg);
	arg_count += 1;
    }
    return result;
}


// ===========================================================================
// To manage a command line on Windows, it is necessary to transform a
// single string into a list of strings.  The arguments found in cmdline
// are appended to the given cmdargs list.
//
// Backslashes and double-quotes are respected.
// 
void MBDriver::determine_command_arguments(const string &cmdline,
					   list<string> &cmdargs) {
    const char *p = cmdline.c_str();
    const char *start = NULL;
    string arg;
    bool in_quote = false;
    for (;;) {
	// p points to the current command line character being examined.
	// start indicates the beginning of the string which needs to be
	// copied to the argument.
        const char ch = *p;
        if (start != NULL && (ch == '\0' || (ch == ' ' && !in_quote))) {
	    arg.append(start, p - start);
	    cmdargs.push_back(arg);
	    start = NULL;
	}
	if (ch == '\0') {
	    break;
	}
	if (start == NULL && !(ch == ' ' || ch == '\t')) {
	    start = p;
	    arg = "";
	}
	if (start == NULL) {
	    // Wait for a non-whitespace character.
	}
	else if (ch == '\\' && p[1] == '"') {
	    arg.append(start, p - start);
	    start = p + 1;
	    p = start;
	}
	else if (ch == '"') {
	    in_quote = !in_quote;
	    arg.append(start, p - start);
	    start = p + 1;
	}
	p += 1;
    }
}

#endif // defined(_WIN32)
