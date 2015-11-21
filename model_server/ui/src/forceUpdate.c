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
/*
 * forceUpdate.c
 *
 * Ensure that a widget's contents are visible  before returning.
 *
 * Adapted from "Motif Programming Manual" by Dan Heller, pp. 687-688
 */


#ifdef _WIN32
typedef char *caddr_t;
#endif

#include <X11/Intrinsic.h>


extern void driver_client_hook_C ();

void forceUpdate(w)
    Widget w;
{
#ifndef NEW_UI
    XtAppContext cxt = XtWidgetToApplicationContext(w);
    Widget diashell;

    /* Locate the shell we are interested in. */
    for(diashell = w; !XtIsShell(diashell); diashell = XtParent(diashell)) {}

    /* If the dialog shell is not realized, don't bother. */
    if(XtIsRealized(diashell))
    {
	Display* dpy = XtDisplay(diashell);
	Window diawindow = XtWindow(diashell);
	XWindowAttributes xwa;

	/* Wait for the dialog to be mapped.  It's guaranteed to become so. */
	while(XGetWindowAttributes(dpy, diawindow, &xwa) &&
	      xwa.map_state != IsViewable)
	{
	    XEvent event;

	    /* We are guaranteed there will be an event of some kind. */
	    XtAppNextEvent(cxt, &event);
	    
	    if (event.type == ClientMessage)
		driver_client_hook_C (&event);
	    else
		XtDispatchEvent(&event);
	}
    }

    /* The next XSync() will get an expose event. */
    XmUpdateDisplay(diashell);
#endif
}

/*
   START-LOG-------------------------------------------

   $Log: forceUpdate.c  $
   Revision 1.2 1996/06/28 18:22:09EDT mstarets 
   NT branch merge
 * Revision 1.2.1.3  1993/09/09  23:12:19  boris
 * Fixed synchro problem while parsing
 *
 * Revision 1.2.1.2  1992/10/09  20:13:50  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
