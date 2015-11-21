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
/*_______________________________________________________________________
!
! waiter_types.h.C
!
! Synopsis:	Contains member functions for different waiter classes.
!
_______________________________________________________________________*/


// INCLUDE FILES

#include "machdep.h"
#include <genError.h>
#include <top_widgets.h>
#include <waiter.h>
#include <waiter_types.h>

extern "C" int browser_import_cancel(void* , int);

// FUNCTIONS DEFINITIONS

//------------------------------------------
// dumbWaiter
//------------------------------------------

dumbWaiter::dumbWaiter() : waiter()
{}

dumbWaiter::~dumbWaiter()
{}

void dumbWaiter::start()
{
    Initialize(dumbWaiter::start);
    Return
}

//------------------------------------------
// animatedCursor
//------------------------------------------

animatedCursor::animatedCursor(const char** values)
: waiter(values), cursor_name(values[ANIM_CURSOR_FEEDBACK]), started(false)
{}

animatedCursor::animatedCursor(const char** values, const char* name)
: waiter(values), cursor_name(name), started(false)
{}

animatedCursor::~animatedCursor()
{
    if(started)
	pop_cursor();
}

void animatedCursor::start()
{
    Initialize(animatedCursor::start);

    started = true;
    push_busy_cursor();

    Return
}

void animatedCursor::time_remaining(int)
{
    Initialize(animatedCursor::time_remaining);
    Return
}

void animatedCursor::percent_completed(int)
{
    Initialize(animatedCursor::percent_completed);
    Return
}

//------------------------------------------
// staticCursor
//------------------------------------------

staticCursor::staticCursor(const char** values)
: animatedCursor(values, values[STATIC_CURSOR_FEEDBACK])
{}

staticCursor::~staticCursor()
{}

void staticCursor::time_remaining(int)
{
    Initialize(staticCursor::time_remaining);
    Return
}

void staticCursor::percent_completed(int)
{
    Initialize(staticCursor::percent_completed);
    Return
}

//------------------------------------------
// windowWaiter
//------------------------------------------

windowWaiter::windowWaiter(const char** values, Widget, int estim_time)
: animatedCursor(values, (values[ANIM_CURSOR_FEEDBACK] ?
			  values[ANIM_CURSOR_FEEDBACK] :
			  values[STATIC_CURSOR_FEEDBACK])),
  dialogTitle(values[COMMAND_NAME]),
  labelString(NULL),
  parent_gt(NULL),
  total_time(estim_time),
  prog(NULL)
{}

windowWaiter::windowWaiter(const char** values, gtBase *gtb, int estim_time)
: animatedCursor(values, (values[ANIM_CURSOR_FEEDBACK] ?
			  values[ANIM_CURSOR_FEEDBACK] :
			  values[STATIC_CURSOR_FEEDBACK])),
  dialogTitle(values[COMMAND_NAME]),
  labelString(NULL),
  parent_gt(gtb),
  total_time(estim_time),
  prog(NULL)
{}

windowWaiter::~windowWaiter()
{
    delete prog;
}


/*_______________________________________________________________________
!
! Function:	windowWaiter::start
!
! Synopsis:	Creates a Progress popup to show what percent of a task
!		is completed.
!
_______________________________________________________________________*/ 
void windowWaiter::start()
{
    Initialize(windowWaiter::start);
    animatedCursor::start();

    delete prog;
    if (cancel_callback != NULL)
	prog = new Progress(
	    parent_gt, dialogTitle, labelString, 
	    cancel_callback,
	    cancel_callback_data);
    else
	prog = new Progress(
	    parent_gt, dialogTitle, labelString, browser_import_cancel, NULL);

    percent_completed(1);
    Return
}

void windowWaiter::title(const char* s)
{
    Initialize(windowWaiter::title);
    dialogTitle = s;
    if (prog) {
	prog->set_title(s);
	prog->popup();
    }
    Return
}

void windowWaiter::label(const char* s)
{
    Initialize(windowWaiter::label);
    labelString = s;
    if (prog) {
	prog->set_label(s);
	prog->popup();
    }
    Return
}

void windowWaiter::status(const char* s)
{
    Initialize(windowWaiter::status);
    if (prog) {
	prog->set_status(s);
    }
    Return
}

void windowWaiter::time_remaining(int t)
{
    Initialize(windowWaiter::time_remaining);
    animatedCursor::time_remaining(t);
    if (prog) {
	prog->set_percent(100 * t / total_time);
    }
    Return
}

void windowWaiter::percent_completed(int p)
{
    Initialize(windowWaiter::percent_completed);
    animatedCursor::percent_completed(p);
    if (prog) {
	prog->set_percent(p);
    }
    Return
}

/*
   START-LOG-------------------------------------------

   $Log: waiter_types.h.C  $
   Revision 1.2 1996/06/28 18:23:20EDT mstarets 
   NT branch merge
Revision 1.2.1.4  1993/02/22  22:23:40  oak
Fixed problem with cancel button.

Revision 1.2.1.3  1992/10/23  17:44:35  glenn
Ignore parent_widget parameter.

Revision 1.2.1.2  92/10/09  20:14:03  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
