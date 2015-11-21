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
#include vstdioHEADER
#include vstdlibHEADER

#include "Application.h"
#include "Args.h"

#ifdef _WIN32
#include <windows.h>

HANDLE event_to_signal;
char   event_to_signal_name[1024];
char   parent_pid[20];
unsigned long parent_exit_wait( unsigned long *);
#endif

int  pid_to_send_signal_to = 0;
int  signal_to_send        = 0;

#define SERVER
#include "DIS_cancel.def"


int main (int argc, char**argv)
{
    
    vdebugTraceEnter(main);
    vstartup(argc, argv);   // Delayed startup of classes.

    Application::ShowTaskbarWindow(0); //hides application's taskbar window on NT

    DIS_args = new Args(argc, argv);
    const char* group = Application::getGroupName();
 
    Application* DIScancel_app;
    extern Server DIScancelServer;
    void DIScanceServerInit();
 
    DIScancel_app = new Application;
    if (DIScancel_app->registerService (group, "DIScancel", DIScancelServer)) {
        fprintf (stderr, "Cannot register service %s:DIScancel; exiting.\n", group);
        exit (EXIT_FAILURE);
        return EXIT_FAILURE;
    }  else
        fprintf (stderr, "Service  %s:DIScancel available.\n", group);

#ifdef _WIN32
	DIS_args->findValue("-signal", event_to_signal_name);
  DIS_args->findValue("-pid", parent_pid);
	event_to_signal = OpenEvent(EVENT_MODIFY_STATE, FALSE, event_to_signal_name);
  unsigned long nId = atol(parent_pid) ;
  unsigned long nNewThreadId;
  CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)parent_exit_wait, 
              (void *)&nId, 0, &nNewThreadId ); 
#else
    char pid_arg[1000];
    char signal_to_send_arg[1000];

    DIS_args->findValue("-signal", signal_to_send_arg);
    if(signal_to_send_arg[0] == 0)
	fprintf(stderr, "DIS_cancel: incorrect or no signal specified.\n");
    else 
	signal_to_send = atoi(signal_to_send_arg);
    
    DIS_args->findValue("-pid", pid_arg);
    if(pid_arg[0] == 0)
	fprintf(stderr, "DIS_cancel: incorrect or no pid specified.\n");
    else
	pid_to_send_signal_to = atoi(pid_arg);
#endif
 
    vevent::Process();

    vdebugTraceLeave ();
    exit (EXIT_SUCCESS);
    return EXIT_FAILURE;
}
              
#ifdef _WIN32
unsigned long parent_exit_wait( unsigned long *pnId)
{
  //The purpose of this function is to detect that parent process finished
  //and call vevent::StopProcessing() which will terminate an event loop
  //and result in termination of a current process
  HANDLE hParent = OpenProcess( PROCESS_ALL_ACCESS, FALSE, *pnId );
  if( hParent )
  {
    WaitForSingleObject(hParent, INFINITE );
    CloseHandle( hParent );
    vevent::StopProcessing();
    veventCancelWaitProc cancel_wait_proc = vevent::GetCancelWaitProc();
    if( cancel_wait_proc )
      cancel_wait_proc();
  }
  ExitThread( 0 );
  return 0;
}

typedef DWORD (WINAPI * UT32PROC)(LPVOID lpBuff, DWORD dwUserDefined,
	LPVOID *lpTranslationList);

typedef BOOL (WINAPI * PUTREGISTER)(HANDLE hModule, LPCSTR SixteenBitDLL,
	LPCSTR InitName, LPCSTR ProcName, UT32PROC* ThirtyTwoBitThunk,
	FARPROC UT32Callback, LPVOID Buff);

typedef VOID (WINAPI * PUTUNREGISTER)(HANDLE hModule);

extern "C" {
static PUTUNREGISTER UTUnRegister = NULL;
static HINSTANCE tclInstance;	/* Global library instance handle. */
static int tclPlatformId;	/* Running under NT, 95, or Win32s? */
}

extern "C"  {

typedef struct LibraryList {
    HINSTANCE handle;
    struct LibraryList *nextPtr;
} LibraryList;

static LibraryList *libraryList = NULL;	/* List of currently loaded DLL's.  */

HINSTANCE
TclWinGetTclInstance()
{
    return tclInstance;
}

int		
TclWinGetPlatformId()
{
    return tclPlatformId;
}

static void
UnloadLibraries()
{
    LibraryList *ptr;

    while (libraryList != NULL) {
	FreeLibrary(libraryList->handle);
	ptr = libraryList->nextPtr;
	ckfree((char*)libraryList);
	libraryList = ptr;
    }
}

extern "C" void TclWinConvertError(DWORD errCode);

HINSTANCE
TclWinLoadLibrary(char *name)
{
    HINSTANCE handle;
    LibraryList *ptr;

    handle = LoadLibrary(name);
    if (handle != NULL) {
	ptr = (LibraryList*) ckalloc(sizeof(LibraryList));
	ptr->handle = handle;
	ptr->nextPtr = libraryList;
	libraryList = ptr;
    } else {
	TclWinConvertError(GetLastError());
    }
    return handle;
}

int 
TclWinSynchSpawn(void *args, int type, void **trans, Tcl_Pid *pidPtr)
{
    static UT32PROC UTProc = NULL;
    static int utErrorCode;

    if (UTUnRegister == NULL) {
	/*
	 * Load the Universal Thunking routines from kernel32.dll.
	 */

	HINSTANCE hKernel;
	PUTREGISTER UTRegister;
	char buffer[] = "TCL16xx.DLL";

	hKernel = TclWinLoadLibrary("Kernel32.Dll");
	if (hKernel == NULL) {
	    return 0;
	}

	UTRegister = (PUTREGISTER) GetProcAddress(hKernel, "UTRegister");
	UTUnRegister = (PUTUNREGISTER) GetProcAddress(hKernel, "UTUnRegister");
	if (!UTRegister || !UTUnRegister) {
	    UnloadLibraries();
	    return 0;
	}

	/*
	 * Construct the complete name of tcl16xx.dll.
	 */

	buffer[5] = '0' + TCL_MAJOR_VERSION;
	buffer[6] = '0' + TCL_MINOR_VERSION;

	/*
	 * Register the Tcl thunk.
	 */

	if (UTRegister(tclInstance, buffer, NULL, "UTProc", &UTProc, NULL,
		NULL) == FALSE) {
	    utErrorCode = GetLastError();
	}
    }

    if (UTProc == NULL) {
	/*
	 * The 16-bit thunking DLL wasn't found.  Return error code that
	 * indicates this problem.
	 */

	SetLastError(utErrorCode);
	return 0;
    }

    UTProc(args, type, trans);
    *pidPtr = 0;
    return 1;
}

}

#endif
