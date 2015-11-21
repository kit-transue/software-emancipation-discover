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
// gtShellXm.C
//------------------------------------------
// synopsis:
// OSF Motif implementations of GT Shell functions
//
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtShellXm.h>

#include <logo50.xbm>

static Pixmap iconPix  = NULL;
static Pixmap iconMask = NULL;

// EXTERNAL FUNCTIONS

extern "C" void add_top_widget(Widget);
extern "C" void rem_top_widget(Widget);
extern "C" void reset_current_cursor();


// VARIABLE DEFINITIONS

static int application_has_focus;


// FUNCTION DEFINITIONS

static void forceUpdate(Widget w)
{
    XtAppContext cxt = XtWidgetToApplicationContext(w);

    // Locate the shell we are interested in.
    Widget diashell;
    for(diashell = w; !XtIsShell(diashell);
	diashell = XtParent(diashell)) {}

    // If the dialog shell is not realized, dont bother.
    if(XtIsRealized(diashell))
    {
	Display* dpy = XtDisplay(diashell);
	Window diawindow = XtWindow(diashell);
	XWindowAttributes xwa;

	// Wait for the dialog to be mapped.  It is guaranteed to become so.
	while(XGetWindowAttributes(dpy, diawindow, &xwa) &&
	      xwa.map_state != IsViewable)
	{
	    XEvent event;

	    // We are guaranteed there will be an event of some kind.
	    XtAppNextEvent(cxt, &event);
	    XtDispatchEvent(&event);
	}
    }

    // The next XSync() will get an expose event.
    XmUpdateDisplay(diashell);
}

#ifdef VERBOSE_FOCUS
#include <stdio.h>
#endif //VERBOSE_FOCUS

static void handle_focus(Widget, XtPointer, XEvent* e, Boolean*)
{
    if(e->xany.type == FocusIn  || e->xany.type == FocusOut)
    {
	XFocusChangeEvent& event = e->xfocus;

#ifdef VERBOSE_FOCUS
	OSapi_fprintf(stderr, "%s  %s  %d  ->  ",
		(event.type == FocusIn ? "In " : "Out"),
		(event.mode == NotifyNormal ? "Normal" :
		 (event.mode == NotifyGrab   ? "Grab  " :
		  (event.mode == NotifyUngrab ? "Ungrab" :
		   "OTHER"))),
		application_has_focus);
#endif //VERBOSE_FOCUS

	if(application_has_focus == 2)
	{
	    if(event.mode == NotifyUngrab)
		application_has_focus = (event.type == FocusIn);
	}
	else
	{
	    if(event.mode == NotifyGrab)
		application_has_focus = 2;
	    else
		application_has_focus = (event.type == FocusIn);
	}

#ifdef VERBOSE_FOCUS
	OSapi_fprintf(stderr, "%d\n", application_has_focus);
#endif //VERBOSE_FOCUS
    }
}


extern "C" void gtShell_init_focus_handler(void* w)
{
    XtAddEventHandler(Widget(w), FocusChangeMask, False, handle_focus, NULL);
}


extern "C" void gtShell_set_focus(void* w)
{
    // According to the ICCCM, applications should not set the input focus
    // unless one of their windows already has it.

    if(application_has_focus)
	XSetInputFocus(XtDisplay(Widget(w)), XtWindow(Widget(w)),
		       RevertToParent, CurrentTime);
}


void gtShell::destroy_init()
{
    add_top_widget(r->widget());
    gtBase::destroy_init();
    XtAddCallback(r->widget(), XmNdestroyCallback,
		  XtCallbackProc(rem_top_widget), NULL);

    gtShell_init_focus_handler(r->widget());
}

extern "C" void* gtShell_icon_pixmap()
{
    return (void*)iconPix;
}

extern "C" void* gtShell_icon_mask()
{
    return (void*)iconMask;
}

void gtShell::icon_title (const char* title)
{
    if(title)
    {
        Arg args[1];
        XtSetArg(args[0], XmNiconName, title);
        XtSetValues(r->widget(), args, 1);
    }
}

void gtShell::popup()
{
    if (!XtIsRealized(r->widget()))
	XtRealizeWidget(r->widget());
    if (!iconPix)
	iconPix = XCreatePixmapFromBitmapData(
	    XtDisplay(r->widget()), XtWindow(r->widget()),
	    logo50_bits, logo50_width, logo50_height, 1, 0, 1);
    if (!iconMask)
	iconMask = XCreatePixmapFromBitmapData(
	    XtDisplay(r->widget()), XtWindow(r->widget()),
	    logo50_bits, logo50_width, logo50_height, 1, 0, 1);
    XtVaSetValues(r->widget(), XmNiconPixmap, iconPix, XmNiconMask, iconMask, NULL);
    
    XtPopup(r->widget(), XtGrabNone);
    forceUpdate(r->widget());
    reset_current_cursor();
    bring_to_top();
}


void gtShell::popdown()
{
    if(r->widget())
    {
	XtUnmanageChild(r->widget());
	XtPopdown(r->widget());
    }
}


inline void reconfigure_rep(
    gtRep* r, unsigned int mask, XWindowChanges& changes)
{
    XReconfigureWMWindow(XtDisplay(r->widget()), XtWindow(r->widget()),
			 DefaultScreen(XtDisplay(r->widget())),
			 mask, &changes);
}


void gtShell::bring_to_top()
{
/*
    unsigned int mask = CWStackMode;

    XWindowChanges changes;
    changes.stack_mode = TopIf;

    reconfigure_rep(r, CWStackMode, changes);
*/

    XMapRaised (XtDisplay(r->widget()), XtWindow(r->widget()));

    gtShell_set_focus(r->widget());
}


void gtShell::move(int x, int y)
{
    XWindowChanges changes;
    changes.x = x;
    changes.y = y;

    reconfigure_rep(r, CWX|CWY, changes);
}


void gtShell::resize(int w, int h)
{
    XWindowChanges changes;
    changes.width = w;
    changes.height = h;

    reconfigure_rep(r, CWWidth|CWHeight, changes);
}


void gtShell::move_resize(int x, int y, int w, int h)
{
    XWindowChanges changes;
    changes.x = x;
    changes.y = y;
    changes.width = w;
    changes.height = h;

    reconfigure_rep(r, CWX|CWY|CWWidth|CWHeight, changes);
}


/*
   START-LOG-------------------------------------------

   $Log: gtShellXm.h.C  $
   Revision 1.4 2000/07/07 08:10:49EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.15  1994/07/05  20:44:28  kws
Bug track: 7637
Implement ability to set icon name - and fix browser and viewer icons

Revision 1.2.1.14  1994/03/10  16:50:23  jon
Bug track: 6624
Set cursor to pointer when cursor stack is dry instead of
setting cursor to "None". Added hook into gtShellXm.h.C
to reset the cursor when gt dialogs are popped up
(the reset in add_top_widgets is too early, the widget
has not been realized at that point).

Revision 1.2.1.13  1994/02/23  20:27:41  jon
Realized shell widget before trying to set an icon for it (bug 6501)

Revision 1.2.1.12  1994/02/22  21:06:01  jon
Bug track: 6309
Changed creation of icon pixmap to use XCreatePixmapFromData instead of using
gtBitmap. Moved creation of pixmap from main into gt itself.

Revision 1.2.1.11  1993/12/17  16:22:15  jon
Bug track: 5090
Added code to gtShell to set a window manager hint as to what the
bitmap on our icon should be. Also added code to viewerShell.if for the
same reason.

Revision 1.2.1.10  1993/12/17  16:09:00  jon
Bug track: 5090
Added code to gtShell to set a window manager hint as to what the
bitmap on our icon should be. Also added code to viewerShell.if for the
same reason.

Revision 1.2.1.9  1993/12/06  22:34:25  kws
Bug track: 5427
Make bring_to_top deiconify also

Revision 1.2.1.8  1993/04/30  01:38:48  glenn
Fix bug #3228 by calling XSetInputFocus during bring_to_top.
Add gtShell_init_focus_handler, gtShell_set_focus for use
within gtShell and by UIM/X interfaces (viewerShell).
Keep track of applications ownership of focus in static
variable "application_has_focus".  Try to handle funky
shifting of input focus during grabs in handle_focus callback.
Check for non-NULL widget in popdown.
Make reconfigure_rep inline.

Revision 1.2.1.7  1993/04/19  14:05:37  glenn
Call bring_to_top in popup().  Fixes bug #3183.

Revision 1.2.1.6  1993/01/26  04:59:06  glenn
Remove gtBaseXm.h

Revision 1.2.1.5  1993/01/20  03:38:02  glenn
Add move, resize, move_resize.

Revision 1.2.1.4  1992/10/26  13:03:24  oak
Commented out XtManageChild call that was
causing the filter list dialog to exit.

Revision 1.2.1.3  92/10/23  17:38:50  glenn
Pull forceUpdate into this file as a static function.

Revision 1.2.1.2  92/10/09  19:03:52  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
