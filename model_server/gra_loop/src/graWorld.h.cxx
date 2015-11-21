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
// include files

#include "Object.h"

#include <fcntl.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#endif /* ISO_CPP_HEADERS */
#ifndef hp10
#include <sysent.h>
#endif

#include <general.h>
#include <InterViews/enter-scope.h>
#include <InterViews/world.h>
#include <InterViews/event.h>
#include <InterViews/transformer.h>
#include <InterViews/rubcurve.h>
#include <InterViews/Graphic/instance.h>
#include <InterViews/Graphic/picture.h>
#include <InterViews/Graphic/instance.h>
#include <InterViews/Graphic/ellipses.h>
#include <InterViews/Graphic/label.h>
#include <InterViews/Graphic/splines.h>
#include <InterViews/Graphic/polygons.h>
#include <InterViews/Graphic/ppaint.h>
#include <InterViews/leave-scope.h>
#include <ivfix.h>

#include <genError.h>
#include <genString.h>
#include <systemMessages.h>
#include <top_widgets.h>
#include <gtBase.h>

#include <genTry.h>
#include <graEnums.h>
#include <graWorld.h>
#include <graSymbol.h>
#include <graTools.h>

#include <cmd.h>

#include <machdep.h>
#include <transaction.h>
#include <driver.h>
#include <xxinterface.h>

extern XtAppContext UxAppContext;

//-----------------------------------------------------------------------------

#ifdef NEW_UI

extern "C" void galaxyYield (void);
extern "C" bool isServer (void);

class galaxyHook {
    public:
        galaxyHook (void);
        ~galaxyHook (void);
    private:
        XtIntervalId _id;
        static const unsigned long _ms;
        static void _cb (XtPointer, XtIntervalId*);
};
 
const unsigned long galaxyHook::_ms = 100UL;
 
galaxyHook::galaxyHook (void)
{
    if (isServer())
        _id = XtAppAddTimeOut (UxAppContext, _ms, _cb, (XtPointer) this);
}
 
galaxyHook::~galaxyHook (void)
{
    if (isServer())
        XtRemoveTimeOut (_id);
}
 
void galaxyHook::_cb (XtPointer ctx, XtIntervalId*)
{
    galaxyHook* phook = (galaxyHook*) ctx;
    if (phook == NULL)
        return;
 
    galaxyYield ();
 
    phook->_id = XtAppAddTimeOut (UxAppContext, _ms, _cb, (XtPointer) phook);
}

#endif

//-----------------------------------------------------------------------------

void init_gt_flush();
void logger_flush_transactions();

int is_alt_event_set();

// props and options are description of InterView properties and options.
// Currently, there are no properties and options so these variables describe
// empty lists.

static iv3(PropertyData) props[] = {
    { 0 }
};

static iv3(OptionDesc) options[] = {
    { 0 }
};

//------------------------------------------
// Constructor graWorld
//
// This is the constructor for creating a graphics "world" context. This 
// context is used to initialize InterViews and for managing reading and 
// retrieving symbols. 
//------------------------------------------

graWorld::graWorld( int argc, char *argv[] )
{
#ifndef NEW_UI
    world = new iv3(World) ("DrawParaSET", argc, argv, options, props);

    // Initialize standard persistent attributes
    InitPPaint();
   
    graSymbolArray = NULL;
    load_symbols ();
#endif

    init_gt_flush();
}

extern void bring_error_messages_on_top(void);
extern void bring_modal_dialog_to_top(void);

//------------------------------------------
// run
//
// Main event loop: Reads events from the X server and dispatches them
// to either the X Intrinsics or InterViews handlers.  Catches exceptions
// and continues to run until the "done" variable is set.
//------------------------------------------

void graWorld::run(const int& done)
{
    Initialize (graWorld::run);

    while(!done) {
	genError* err;
        genTry {
          start_transaction(){
	      cmd_begin();
	      gtBase::flush_output();
	      cmd_end();

	      logger_flush_transactions();

	      iv3(Event) e;
#ifdef NEW_UI
	      galaxyHook gh;
#endif
	      world->read(e);
	      if (!(is_alt_event_set()))
		  e.handle();
	      
	      if(e.type() == 2 || e.type() == 3) { // For mouse clicks, try to popup messages.
		  bring_error_messages_on_top();
		  bring_modal_dialog_to_top();
		}

	      cmd_exec_continue();
	      if (cmd_break()) 
		  driver_instance->stop_running();
	      
          } end_transaction();
        } genCatch(err) {

	    reset_cursor();	// Prevent cursor from getting stuck in
				// non-default state.

	} genEndtry
	_lhb();
    }
}

int gra_epoch_test_done (void *flag_ptr) {
    int done = *(int*)flag_ptr;
    return done;
}

void graWorld::run_for_epoch(const int& done)
{
    Initialize (graWorld_run_for_epoch);

    start_transaction(){
	gtBase::epoch_take_control(&gra_epoch_test_done, (void *)&done);
    } end_transaction();
#if 0
    while(!done) {
	start_transaction(){
	    iv3(Event) e;
	    world->read(e);
	    XEvent xe
            if (!(is_alt_event_set())) {
		int tp = e.type();
		if (tp != 2 && tp != 3)      // Ignoring Mouse clicks (up *and* down)
		    e.handle();
	        else
		    OSapi_fprintf (stderr, "\007Ignore event\007\n");
	    }
	} end_transaction();
    }
#endif
}

// Processes all pending events in the queue and returns.

extern Display* UxDisplay;

void graWorld::process_pending_events()
{
    Initialize (graWorld::process_pending_events);

    int e_type;
    while(e_type = XtAppPending(UxAppContext)) {
	genError* err;
        genTry {

	    if (!(e_type & XtIMXEvent)) // No XEvents
	    {
		XtAppProcessEvent(UxAppContext, (XtIMAll ^ XtIMXEvent));
	    }
	    else // At least one XEvent
	    {
		cmd_begin();
		gtBase::flush_output();
		cmd_end();
//	        XSync(UxDisplay, 0);

		iv3(Event) e;
		world->read(e);
		e.handle();
	    }

        } genCatch(err) {

	    reset_cursor();	// Prevent cursor from getting stuck in
				// non-default state.
	} genEndtry
    }
}

//------------------------------------------
// insert_window
//
// Given an Interactor*, insert it into the world.
//------------------------------------------

void graWorld::insert_window( iv3(Interactor) *window )
{
    world->InsertApplication (window, 0, world->Height(), TopLeft);
}

//------------------------------------------
// remove_window
//
// Given a graWindow, remove it from the world
//------------------------------------------

void graWorld::remove_window( iv3(Interactor)* window )
{
    world->Remove (window);
}

//------------------------------------------
// flush
//
// Make sure all world events are flushed to the server
//------------------------------------------

void graWorld::flush()
{
    world->Flush ();
}

/*
// graWorld
//------------------------------------------
// synopsis:
// Implements the graphics environment including a symbol reader
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
*/

