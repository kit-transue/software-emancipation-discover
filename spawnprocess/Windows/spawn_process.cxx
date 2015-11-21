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
//
//This executable is used to spawn processes in a new process group
//and wait for event object to be set. When the main application sets the 
//event object it kill its process group(all processes in the group). 
//This is the method suggested at the Microsoft support site since Win32
//doesnot offer an easy way to terminate child processes.
//Here's the outline of the algorithm:
//1. The main application spawns this executable in a new process group and
//   this executable will be responsible for spawning any further processes.
//   The main application also creates an event object which will act as
//   communication between the two.
//2. This executable receives as its command line argument the name of the
//   process it has to spawn. It spawns off the new process and waits for
//   process to end or for the event object to be set.
//3. When main application decides its spawned processes should terminate,
//   it sets the event object.
//4. This executable on noticing that the event object is set, sends a
//   ctrl-break event to terminate all the processes in its group.
//

#define STRICT
#include <Windows.h>
#include <process.h>
#include <tchar.h>
#include <stdio.h>
#include <list>
#include <string>
#include <ctype.h>
#include "startprocess.h"
using namespace MBDriver;
namespace std {};
using namespace std;

///////////////////////////////////////////////////////////////////////////////
bool is_bat(const char *fn) {
    int l = strlen(fn);
    if (l < 4) {
	return false;
    }
    const char *p = fn + (l - 4);
    return p[0] == '.' && toupper(p[1]) == 'B'
	&& toupper(p[2]) == 'A' && toupper(p[3]) == 'T';
}

///////////////////////////////////////////////////////////////////////////////
 
int  main(int argc, TCHAR* argv[]) {
    // Make sure that we've been passed the right number of arguments
    if (argc < 3) {
        _tprintf(
        __TEXT("Usage: %s (InheritableEventHandle) (CommandLineToSpawn)\n"), 
		argv[0]);
        return(0);
    }
 
    int szCmdLineLen = 0;
    for (int i = 2; i < argc; i++) {
	if (argv[i])
	    szCmdLineLen += strlen(argv[i]) + 10;
    }

    // Construct the full command line
    list<string> cmdargs;
    for (int x = 2; x < argc; x++) {
	cmdargs.push_back(argv[x]);
    }
    
    string cmdline;
    if (is_bat(argv[2])) {
	((cmdline += "cmd.exe /C \"") += prepare_command_arguments(cmdargs)) += "\"";
    }
    else {
        cmdline = prepare_command_arguments(cmdargs);
    }

    TCHAR *szCmdLine = new TCHAR[cmdline.length()+1];
    _tcscpy(szCmdLine, cmdline.c_str());
 
    STARTUPINFO         si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    DWORD dwExitCode = 0;
 
    HANDLE h[2];
    h[0] = (HANDLE) _ttoi(argv[1]);  // The inherited event handle
    int pid = _getpid();

    // Spawn the other processes as part of this Process Group
    BOOL f = CreateProcess(NULL, szCmdLine, NULL, NULL, TRUE, 
                           0, NULL, NULL, &si, &pi);
    if (f) {
        CloseHandle(pi.hThread);
        h[1] = pi.hProcess;
	DWORD ret_value = WAIT_TIMEOUT;

        // Wait for the spawned-process to die or for the event
        // indicating that the processes should be forcibly killed.
        switch (ret_value = WaitForMultipleObjects(2, h, FALSE, INFINITE)) {
            case WAIT_OBJECT_0 + 0:  // Force termination
		if (is_bat(argv[2])) {
		    TerminateProcess(pi.hProcess, 1);
		}
                GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid);
                WaitForSingleObject(pi.hProcess, INFINITE);
                break;
 
            case WAIT_OBJECT_0 + 1:  // App terminated normally
                // Make its exit code our exit code
                GetExitCodeProcess(pi.hProcess, &dwExitCode);
                break;   
	}
        CloseHandle(pi.hProcess);
    }
    else {
	dwExitCode = GetLastError();
        printf("Could not create process for: %s\n", szCmdLine);
    }

    CloseHandle(h[0]);   // Close the inherited event handle

    if(szCmdLine)
	delete szCmdLine;
    return(dwExitCode);
}

//////////////////////////////// End of File //////////////////////////////////
