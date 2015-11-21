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
#include <vport.h>
#include vstartupHEADER

#include <galaxy_undefs.h>
#include <cLibraryFunctions.h>
#include <machdep.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <genString.h>
#include <customize.h>
#include <vpopen.h>
#include <ParaCancel.h>
#ifdef _WIN32
#include <windows.h>
#endif

ParaCancel* ParaCancel::canceller = NULL;
static ParaCancel* cancel_server  = NULL;

#ifdef _WIN32
HANDLE cancel_event_handle;

unsigned long __stdcall ParaCancel::watch_cancel_event(void *)
{
	while(1){
		WaitForSingleObject(cancel_event_handle, INFINITE);
		if (canceller) canceller->cancelled = 1;
		ResetEvent(cancel_event_handle);
	}	
	return 0;
}

#endif 

ParaCancel::ParaCancel(const char *group)
{
    genString executable; 

#ifdef _WIN32
		DWORD thread_id;
		sig       = 0;
    pid       = OSapi_getpid();
    cancelled = 0;

		genString event_name;
		event_name.printf("DISCOVER_CANCEL_EVENT%d", pid);
		cancel_event_handle = CreateEvent(NULL, TRUE, FALSE, event_name);
		CreateThread(NULL, 4096, watch_cancel_event, NULL, 0, &thread_id);
    executable.printf ("%s/bin/DIS_cancel.exe -group \"%s\" -signal %s -pid %d",
		       customize::install_root(), group, event_name, pid);
    // we redirect child's stdout. Child process will terminate itself
    // when this pipe breaks
    fd = vpopen (executable, "w");
    if(fd == NULL){
			printf("Can not start DIS_cancel service\n");
			fflush(stdout);
    }
#else
    sig       = SIGUSR1;
    pid       = OSapi_getpid();
    cancelled = 0;

    executable.printf ("%s/bin/DIS_cancel -group \"%s\" -signal %d -pid %d",
		       customize::install_root(), group, sig, pid);

    signal (sig, (SIG_TYP) &ParaCancel::cancel);
    fd = vpopen (executable, "-r");
    if(fd == NULL){
	printf("Can not start DIS_cancel service\n");
	fflush(stdout);
    }
#endif
    cancel_server = this;

}

ParaCancel::~ParaCancel()
{
}

void ParaCancel::set(void)
{
    if(cancel_server){
	canceller            = cancel_server;
	canceller->cancelled = 0;
    }
}

void ParaCancel::reset(void)
{
    canceller = NULL;
}

void ParaCancel::cancel(int)
{
    if (canceller) canceller->cancelled = 1;
}

