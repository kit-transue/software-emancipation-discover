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
// main.C : main entry point for the model build driver console app
//
//  1) Start the Java-based model build GUI.
//  2) Start the listener which serves a socket to the GUI.
//  3) Start the listener which serves a socket to log presentation.
//  Once the GUI socket connects, and directs to build the model,
//     1) Start the log presentation process.
//     2) Start the model build Perl script.
//  Then,
//     1) If model build completes, notify the GUI and if necessary terminate
//        the log presentation process.
//     2) If the GUI cancels model build, send a SIGINT to the Perl script
//        model build process.  Also if necessary terminate
//	  the log presentation.
//        Wait for completion, then send <done/> to the GUI.
//     3) If the GUI terminates, cancel the model build, etc., and exit.
//
//  ** Threads **
//  This program relies on threads to wait for several events simultaneously.
//  1) termination of the Java GUI (listener thread)
//  2) termination of the Perl model build script.
//  3) termination of log presentation
//  4) listen for new connection from the GUI
//  5) listen for new connection from log presentation
//  6) message from GUI
//  7) message from log presentation
//  The thread which receives log messages also passes them through
//  to the GUI's connected socket.  The message-receiving threads are
//  started by the Transport communications system.
//

#include <string.h>
#include <stdlib.h>
#include "startprocess.h"
#include "mbdriver_util.h"
#include "util.h"
#include "guidriver.h"
#include "lpdriver.h"
#include "msg.h"
#include "svcnames.h"
#include "threads.h"

#ifdef _WIN32
#include <winmain.h>
#endif //_WIN32

namespace std {}
using namespace std;
using namespace MBDriver;

static bool test_no_gui = false;

// ===========================================================================
bool check_args(int argc, char *argv[], const char *&proj_name,
					const char *&proj_file,
					const char *&gui_service_name) {
    bool bad = false;
    for (int i = 1; i < argc; i += 1) {
	if (argv[i] == NULL) {
	    bad = true;
	}
	else if (strcmp(argv[i], "-projectName") == 0
	         && i+1 < argc && argv[i+1] != NULL && proj_name == NULL) {
	    proj_name = argv[i+1];
	    i += 1;
	}
	else if (strcmp(argv[i], "-projectFile") == 0
	         && i+1 < argc && argv[i+1] != NULL && proj_file == NULL) {
	    proj_file = argv[i+1];
	    i += 1;
	}
	else if (strcmp(argv[i], "-log_windows") == 0) {
	    enable_log_windows();
	}
	else if (strcmp(argv[i], "-debug_messages") == 0
	          || strcmp(argv[i], "-trace_sends") == 0) {
	    trace_sends = true;
	}
	else if (strcmp(argv[i], "-trace_receives") == 0) {
	    trace_receives = true;
	}
	else if (strcmp(argv[i], "-gui_service") == 0
	         && i+1 < argc && argv[i+1] != NULL && gui_service_name == NULL) {
	    gui_service_name = argv[i+1];
	    i += 1;
	    if (test_no_gui) {
		bad = true;
	    }
	}
	else if (strcmp(argv[i], "-test_no_gui") == 0) {
	    test_no_gui = true;
	    if (gui_service_name != NULL) {
		bad = true;
	    }
	}
	else {
	    bad = true;
	}
    }
    if (bad) {
	msg("usage: mbdriver [-gui_service <service_name>]", catastrophe_sev) << eom;
    }
    return !bad;
}

// ===========================================================================
#ifdef _WIN32
WINMAIN()
#else //not win32
int main(int argc, char* argv[]) {
#endif // win32
    const char *proj_name = NULL;
    const char *proj_file = NULL;
    const char *gui_service_name = NULL;
    if (!check_args(argc, argv, proj_name, proj_file, gui_service_name)) {
	return 1;
    }

#ifdef _WIN32
	// check if we need to allocate a console for the program
	if(is_log_windows_enabled()) CreateConsole();

#endif

    SAThreads::initialize();

    if (trace_sends || trace_receives) {
	dbgprt("");
	dbgprt("");
	dbgprt("starting mbdriver");
    }

    if (gui_service_name != NULL) {
	set_gui_service_name(gui_service_name);
    }

    // Initialize PSETHOME.
    psethome();

    if (!test_no_gui) {
	run_gui_process(proj_name, proj_file, gui_service_name == NULL);
    }
    else {
	run_log_presentation_test();
    }
    return 0;
}

