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
// gtScrollWinXm
//------------------------------------------
// synopsis:
// Implentation of member functions for the ScrolledWindow
// class (gtScrolledWindowXm)
//------------------------------------------

// INCLUDE FILES

#include <gtSepar.h>
#include <gtScrollbar.h>
#include <gtBaseXm.h>
#include <gtScrollWinXm.h>

#include <Xm/ScrolledW.h>


// FUNCTION DEFINITIONS

gtScrolledWindow* gtScrolledWindow::create(gtBase *parent, const char* name)
{
    return new gtScrolledWindowXm(parent, name);
}

gtScrolledWindow* gtScrolledWindow::create(gtBase *parent, const char* name,
					   int height, int width)
{
    return new gtScrolledWindowXm(parent, name, height, width);
}

gtScrolledWindowXm::gtScrolledWindowXm(gtBase *parent, const char *name)
{
    Arg		args[1];

    XtSetArg(args[0], XmNscrollingPolicy, XmAUTOMATIC);
    r->widget(XmCreateScrolledWindow(parent->container()->rep()->widget(),
				     (char *)name,args,1));
    destroy_init();
}

gtScrolledWindowXm::gtScrolledWindowXm(gtBase *parent, const char *name,
				       int height, int width)
{
    if(parent)
    {
	Arg args[3];
	int n = 0;

	XtSetArg(args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
	if (height != -1) {
	    XtSetArg(args[n], XmNheight, height); n++; }
	if (width != -1) {
	    XtSetArg(args[n], XmNwidth, width); n++; }
	r->widget(XmCreateScrolledWindow(parent->container()->rep()->widget(),
					 (char*)name, args, n));
    }
    destroy_init();
}

gtScrollbar* gtScrolledWindowXm::horizBar()
{
    gtScrollbar* ans = gtScrollbar::create(NULL, "ans", NULL, NULL, gtHORZ);
    ans->release_rep();

    Widget w;
    Arg args[1];
    XtSetArg(args[0], XmNhorizontalScrollBar, &w);
    XtGetValues(r->widget(), args, 1);

    ans->attach_rep(new gtRep(w));

    return ans;
}

gtScrollbar *gtScrolledWindowXm::vertBar()
{
    gtScrollbar* ans = gtScrollbar::create(NULL, "ans", NULL, NULL, gtVERT);
    ans->release_rep();

    Widget w;
    Arg args[1];
    XtSetArg(args[0], XmNverticalScrollBar, &w);
    XtGetValues(r->widget(), args, 1);

    ans->attach_rep(new gtRep(w));

    return ans;
}


void gtScrolledWindowXm::set_work_area(gtPrimitive* wa)
{
    Arg args[1];
    XtSetArg(args[0], XmNworkWindow, wa->rep()->widget());
    XtSetValues(r->widget(), args, 1);
}


void gtScrolledWindowXm::set_horz_bar(gtScrollbar* sb)
{
    Arg args[1];
    XtSetArg(args[0], XmNhorizontalScrollBar, sb->rep()->widget());
    XtSetValues(r->widget(), args, 1);
}


void gtScrolledWindowXm::set_vert_bar(gtScrollbar* sb)
{
    Arg args[1];
    XtSetArg(args[0], XmNverticalScrollBar, sb->rep()->widget());
    XtSetValues(r->widget(), args, 1);
}


void gtScrolledWindowXm::scrollbar_pos(gtCardinalDir vdir, gtCardinalDir hdir)
{
    Arg	args[1];
    if (vdir == gtTop)
	if (hdir == gtLeft)
	    XtSetArg(args[0], XmNscrollBarPlacement, XmTOP_LEFT);
	else
	    XtSetArg(args[0], XmNscrollBarPlacement, XmTOP_RIGHT);
    else if (hdir == gtLeft)
	XtSetArg(args[0], XmNscrollBarPlacement, XmBOTTOM_LEFT);
    else
	XtSetArg(args[0], XmNscrollBarPlacement, XmBOTTOM_RIGHT);

    XtSetValues(r->widget(),args, 1);
}


/*
   START-LOG-------------------------------------------

   $Log: gtScrollWinXm.h.C  $
   Revision 1.1 1993/01/25 23:53:45EST builder 
   made from unix file
Revision 1.2.1.3  1993/01/26  04:56:25  glenn
Check parent in ctor.

Revision 1.2.1.2  1992/10/09  19:03:47  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
