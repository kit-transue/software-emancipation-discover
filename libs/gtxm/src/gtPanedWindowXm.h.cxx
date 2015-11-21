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
// gtPanedWindowXm
//------------------------------------------
// synopsis:
// Implentation of member functions for the PanedWindow class (gtPanedWindowXm)
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtPanedWindowXm.h>

#include <Xm/PanedW.h>


// FUNCTION DEFINITIONS

gtPanedWindow* gtPanedWindow::create(gtBase* parent, const char* name)
{
    return new gtPanedWindowXm(parent, name, 0, 0);
}


gtPanedWindow* gtPanedWindow::create(
    gtBase* parent, const char* name, int height, int width)
{
    return new gtPanedWindowXm(parent, name, height, width);
}


gtPanedWindowXm::gtPanedWindowXm(
    gtBase* parent, const char* name, int height, int width)
{
    if(parent)
    {
	Arg args[3];
	int n = 0;

	XtSetArg(args[n], XmNallowResize, True); n++;

	if (height > 0) {
	    XtSetArg(args[n], XmNheight, height); n++;
	}

	if (width > 0) {
	    XtSetArg(args[n], XmNwidth, width); n++;
	}

	r->widget(XmCreatePanedWindow(
	    parent->container()->rep()->widget(), (char*)name, args, n));
    }
    destroy_init();
}


/*
   START-LOG-------------------------------------------

   $Log: gtPanedWindowXm.h.C  $
   Revision 1.1 1993/01/25 23:48:07EST builder 
   made from unix file
Revision 1.2.1.4  1993/01/26  04:50:57  glenn
Check parent in ctor.

Revision 1.2.1.3  1993/01/14  19:36:58  glenn
Use parent->container()->rep() instead of parent->rep() in ctor.
Remove simple ctor, prepare.

Revision 1.2.1.2  1992/10/09  19:03:40  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
