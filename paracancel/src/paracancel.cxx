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

// canceltool.C
//------------------------------------------
// synopsis:
// 
//------------------------------------------

// INCLUDE FILES

#define _cLibraryFunctions_h

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <genString.h>
#include <vpopen.h>

#include <gtAppShell.h>
#include <gtPushButton.h>
#include <gtBaseXm.h>

#include <ParaCancel.h>

#include <machdep.h>

// EXTERNAL FUNCTIONS


// VARIABLE DEFINITIONS

#ifdef sun5
struct sigaction act;
struct sigaction oldact;
#endif

#ifdef irix6
#include <errno.h>
#endif /* irix6 */

static char application_name[] = "ParaCANCEL";

ParaCancel* ParaCancel::canceller;

// FUNCTION DEFINITIONS

void ParaCancel::cancel (int)
{
    if (canceller) canceller->cancelled = 1;
}

void application(gtApplicationShell* app_shell, unsigned int argc, char** argv)
{
    new ParaCancel(app_shell, argc, argv);
}

ParaCancel::ParaCancel ( const char* message, int sig, int pid, int win) : 
    cancelled(0), fd(0)
{
    genString executable;

    if (!sig) sig = SIGUSR1;
    if (!pid) pid = OSapi_getpid();

    executable.printf ("%s/bin/paracancel -message \"%s\" -signal %d -pid %d -win %d",
		       getenv("PSETHOME"), message, sig, pid, win);

#ifdef sun5
    act.sa_handler = &ParaCancel::cancel;
    sigfillset( &act.sa_mask );
    act.sa_flags = SA_RESTART;
    sigaction(sig, &act, &oldact);
#else
    signal (sig, (SIG_TYP) &ParaCancel::cancel);
#endif
    fd = vpopen (executable, "-R");

    canceller  = this;
}

ParaCancel::ParaCancel (gtApplicationShell*  app_shell, unsigned int argc, char** argv) : cancelled(0), fd(0)
{
    long win = 0;
    genString message;
    genString color;
    
    pid = OSapi_getppid();
    sig = 15;

    for (int i=1; i<argc; i++) {
	if (strcmp(argv[i], "-win") == 0) {
	    int local_win = atoi (argv[i+1]);
	    if (local_win) win = local_win;
	    i++;
	}
        else if (strcmp(argv[i], "-pid") == 0) {
	    int local_pid = atoi (argv[i+1]);
	    if (local_pid) pid = local_pid;
	    i++;
	}
        else if (strcmp(argv[i], "-signal") == 0) {
	    int local_sig = atoi (argv[i+1]);
	    if (local_sig) sig = local_sig;
	    i++;
	}
        else if (strcmp(argv[i], "-message") == 0) {
	    message = argv[i+1];
	    i++;
        }
    }
    if (message.length() == 0) {
	message = "Cancel operation?";
    }

    shell = gtTopLevelShell::create(app_shell, "ParaCANCEL");
    shell->title(application_name);
    shell->override_WM_destroy(wm_close);

    shell_form = gtForm::create(shell, "window_form");
    shell_form->attach(gtLeft);
    shell_form->attach(gtRight);
    shell_form->attach(gtTop);
    shell_form->attach(gtBottom);
    shell_form->manage();

    if (win) {
        Window root;
        int x, y;
        unsigned int width, height, border_width, depth;

        XtRealizeWidget (shell->rep()->widget());

	gtPushButton* button = gtPushButton::create (shell_form, "ok", message, cancel_CB, this);

	Display* display = XtDisplay(shell->rep()->widget());
        XGetGeometry (display, win, &root, &x, &y, &width, &height, &border_width, &depth);


        int h, w;
        h = gtBase::pix_to_hund_vert(height*10);
        w = gtBase::pix_to_hund_horiz(width*10);
	button->size (h, w);
	button->manage();
	Widget widget = button->rep()->widget();
	XtRealizeWidget (widget);

	Window window = XtWindow(widget);
	XReparentWindow (display, window, win, 0, 0);
        XtMapWidget(widget);

    } else {

	gtPushButton* button = gtPushButton::create (shell_form, "ok", message, cancel_CB, this);
	button->manage();
	shell->popup();
    }


    canceller  = this;
}

ParaCancel::~ParaCancel ()
{
    if (fd)  {
#ifndef irix6
        ::kill (vp_find_pid(fd), 9);
#else
        pid_t pid = vp_find_pid(fd);
        int err = 0, i = 0;
        do {
          usleep(250000);        // quarter of a second
          err = ::kill(pid, 9);
        } while ((err == -1) && (errno == EPERM) && (++i < 16));
#endif /* ! irix6 */
	vpclose(fd);
	fd = 0;
    }
    
    canceller = 0;
}

void ParaCancel::cancel_CB (gtPushButton*, gtEventPtr, void* data, gtReason)
{
    ParaCancel *canceller = (ParaCancel*) data;
    msg("kill: $1 ($2)") << canceller->pid << eoarg << canceller->sig << eom;
    ::kill (canceller->pid, canceller->sig);

    exit(0);
}

int ParaCancel::wm_close(void*)
{
    exit(0);
    return 1;
}


void ParaCancel::set(void)
{
}

void ParaCancel::reset(void)
{
}
