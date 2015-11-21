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
// gtBitmapXm.h.C
//------------------------------------------
// synopsis:
// 
// One-bit deep image.
//------------------------------------------

// INCLUDE FILES

#include <genArr.h>
#include <sys/stat.h>
#include <gtNoParent.h>
#include <gtBitmapXm.h>
#include <Xm/Label.h>



// FUNCTION DEFINITIONS

gtBitmap* gtBitmap::create(
    gtBase* parent, const char* name, const char* bits, int w, int h)
{
    return new gtBitmapXm(parent, name, bits, w, h);
}


gtBitmapXm::gtBitmapXm(
    gtBase* parent, const char* name, const char* bits, int w, int h)
: pixmap(0)
{
    if(parent)
    {
	int n = 0;
	Arg args[10];

	XtSetArg(args[n], XmNlabelType, XmPIXMAP), n++;
	XtSetArg(args[n], XmNmarginWidth, 0), n++;
	XtSetArg(args[n], XmNmarginHeight, 0), n++;
	XtSetArg(args[n], XmNwidth, w), n++;
	XtSetArg(args[n], XmNheight, h), n++;
	XtSetArg(args[n], XmNrecomputeSize, False), n++;

	r->widget(XmCreateLabel(
	    parent->container()->rep()->widget(), (char*)name, args, n));

	Display* display = XtDisplay(r->widget());

	Screen* screen;
	Pixel back_color, fore_color;
	int depth;

	n = 0;
	XtSetArg(args[n], XmNscreen, &screen), n++;
	XtSetArg(args[n], XmNforeground, &fore_color), n++;
	XtSetArg(args[n], XmNbackground, &back_color), n++;
	XtSetArg(args[n], XmNdepth, &depth), n++;

	XtGetValues(r->widget(), args, n);

	if(pixmap = XCreatePixmapFromBitmapData(
	    display, RootWindowOfScreen(screen),
	    (char*)bits, w, h, fore_color, back_color, depth))
	{
	    n = 0;
	    XtSetArg(args[n], XmNlabelPixmap, pixmap), n++;
	    XtSetValues(r->widget(), args, n);
	}
    }
    destroy_init();
}


gtBitmapXm::~gtBitmapXm()
{
    if(pixmap && r->widget())
	XFreePixmap(XtDisplay(r->widget()), pixmap);
}


/*
   START-LOG-------------------------------------------

   $Log: gtBitmapXm.h.C  $
   Revision 1.2 1996/03/24 12:21:52EST rajan 
   isolate system dependencies - I
// Revision 1.3  1994/03/25  13:20:12  builder
// Port
//
// Revision 1.2  1993/06/02  20:28:52  glenn
// Remove Initialize macros.  Remove genError.h.
//
// Revision 1.1  1993/02/05  23:34:39  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

