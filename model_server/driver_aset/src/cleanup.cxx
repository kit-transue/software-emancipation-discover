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

#ifdef _WIN32
#include <io.h>
#endif 

#include <cLibraryFunctions.h>
#include <machdep.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <LanguageController.h>
#include "evalMetaChars.h"
#ifdef SIGHUP
#undef SIGHUP
#endif
#define SIGHUP 1
#ifndef NEW_UI
#include <gtBase.h>
#include <gtDisplay.h>
#include <gtRTL.h>
#include <gtRTL_cfg.h>
#endif
#include "setprocess.h"

#include <autolog.h>
#include <genError.h>
#include <MemSeg.h>
#include <proj.h>
#include <globals.h>

#include "transaction.h"

#ifndef NEW_UI
#include "WinHelp.h"
#endif

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <windows.h>

extern unsigned long gnMainThreadId;
#endif

void ste_interface_quit();

extern ok_to_delete_old_pmod;
extern "C" void aset_reset_signals();
void smt_clear_cpp_out();

#ifndef NEW_UI
extern Display* UxDisplay;
#endif

extern void fast_exit(int exit_status);

//------------------------------------------
// driver_cleanup
//
// Prepare for the end of the world.
//------------------------------------------

static void driver_cleanup()
{
    Initialize(driver_cleanup);

    // terminate configshell, if it exists
    configurator("exit","");
    
    if (builtin_mmgr) {
	for (mem_segments* mms=mem_segments_get_first();mms;mms=mem_segments_get_next(mms))
	    OSapi_unlink(mms->name);
	delete builtin_mmgr;
	builtin_mmgr = 0;
    }

    LanguageController::destroy();

#ifndef NEW_UI
    if (def_table)
      def_table->save_entries();	// Save RTL filters.
#endif

    projNode::terminate();

    smt_clear_cpp_out();         // do the smt cleanup (like deleting /usr/tmp/cpp_xxx files)
    
    LOG_END;			// Terminate logging of error messages.
}


//------------------------------------------
// no-op signal handler
//------------------------------------------

static void ignore_signal(int) {}

//------------------------------------------
// driver_exit
//
// Bring on the end of the world.
//------------------------------------------

static int already_exiting = 0;

extern "C" int driver_exiting() 
{
    return already_exiting;
}

#ifndef NEW_UI
extern void delete_log_file();
#endif

extern "C" void driver_exit(int exit_status)
{

#ifndef NEW_UI  
  delete_log_file();
#endif

#ifdef _WIN32
  unsigned long nCurrentThreadId = GetCurrentThreadId();
  if( gnMainThreadId && nCurrentThreadId != gnMainThreadId ) //this is not a main thread
  {
    PostThreadMessage( gnMainThreadId, WM_QUIT, (WPARAM)exit_status, (LPARAM)exit_status );
    _endthread();
  }
#endif  
    if (already_exiting)
	fast_exit (exit_status);

    already_exiting = 1;

#ifndef NEW_UI
    if(gtDisplay::is_open())
	WinHelp (UxDisplay, "CODEINTEGRITY_HELP.hlp", HELP_QUIT, 0);
#endif

    // in case of crAash, disable copying of potentially corrupted temporary pmod 
    // to permanent.
#ifdef _WIN32
    if (exit_status != __NO_ERROR)
#else
    if (exit_status != NO_ERROR)
#endif
	ok_to_delete_old_pmod = 0;

        driver_cleanup();

    // Shut down Epoch, and ignore SIGHUP from death of process-leader.
#ifndef _WIN32
    OSapi_signal(SIGHUP, (SIG_TYP) ignore_signal);
#endif
    ste_interface_quit();
    aset_reset_signals();

#ifdef _WIN32
    remove_tmp_dir();
#endif

    // Use our own wrapper for exiting
    fast_exit(exit_status);
}


// This makes sure that scratch apps (groups) are cleaned up
// before exiting.
void delete_scratch_apps() {
   Initialize(delete_scratch_apps);

   start_transaction() {
      Obj* al = app::get_app_list();
      Obj* a;
      ForEach(a, *al) {
	 appPtr ap = checked_cast(app, a);
	 if (ap->is_scratch()) {
	    projModulePtr module = app_get_mod(ap);
	    if (module && module->is_home_project()) {
	       module->delete_module(2, 0);
	    }
	 }
      }
   } end_transaction();
}


