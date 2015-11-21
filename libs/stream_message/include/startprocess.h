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
#ifndef STARTPROCESS_H
#define STARTPROCESS_H

#ifdef _WIN32
#include <windows.h>
#else 
#include <sys/types.h>
#endif
#include <string>
#include <list>
namespace std {}
using namespace std;

namespace MBDriver {

// ===========================================================================
// Note that the process IDs passed around by these functions are generally
// valid only within the current process.  On Unix they're valid throughout
// the system, but on Windows they're handles.
//
#ifdef _WIN32
typedef HANDLE pid_t;
const pid_t null_pid = NULL;
typedef DWORD processIdType;
#else 
typedef ::pid_t pid_t;
const pid_t null_pid = 0;
typedef ::pid_t processIdType;
#endif

// ===========================================================================
// This function will start the process and exit.  Returns null_pid to
// indicate failure, pid if successful.  The argv vector does not include
// an element that duplicates the path.
//
extern pid_t start_process(const char *path, const char *const argv[], int detach_flag = 0);

// ===========================================================================
// Similar to start_process function, except takes extra argument process_id
// to get back the PID of the started process. On Unix both the return value 
// and process_id argument are identical but on NT the return value is process handle 
// while process_id argument is process ID.
//
extern pid_t start_process_get_pid(const char *path, const char *const argv[], int detach_flag, processIdType& process_id);
// ===========================================================================
// Call this to re-enable log windows for debugging, particularly on NT.
//
extern void enable_log_windows();
extern bool is_log_windows_enabled();

// ===========================================================================
// This function will block until the given child process terminates.
//
extern int wait_process(pid_t);

// ===========================================================================
// Determine whether the process is alive or has terminated.
//
extern bool is_process_alive(pid_t pid, long *status = 0);

// ===========================================================================
// Request the given child process to terminate.
//
extern void terminate_process(pid_t pid, bool process_group = 0);

// ===========================================================================
// Execute the path, returning when the child process is done.
//
extern int start_process_and_wait(const char *path, const char *const argv[],
			          string *output = 0);

// ===========================================================================
// This process's pid.
//
extern pid_t get_pid();

// ===========================================================================
// To pass command arguments, it is necessary to transform a
// list of strings into a single string.
// 
extern string prepare_command_arguments(const list<string> &);

// ===========================================================================
// To manage a command line, it is necessary to transform a
// single string into a list of strings.
// 
extern void determine_command_arguments(const string &, list<string> &);

}

#endif // STARTPROCESS_H
