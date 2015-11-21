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
// gtScrollbarXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtScrollbar class in OSF Motif
//------------------------------------------

// INCLUDE FILES

#include <gtSepar.h>
#include <gtBaseXm.h>
#include <gtScrollbarXm.h>

#include <Xm/ScrollBar.h>


// FUNCTION DEFINITIONS

gtScrollbar* gtScrollbar::create(
    gtBase *parent, const char* name,
    gtScrollCB callback, void *client_data, gtDirection dir)
{
    return new gtScrollbarXm(parent, name, callback, client_data, dir);
}


gtScrollbarXm::gtScrollbarXm(
    gtBase* parent, const char* name,
    gtScrollCB callback, void* client_data, gtDirection dir)
{
    if(parent)
    {
	Arg args[2];
	if(dir == gtHORZ)
	    XtSetArg(args[0], XmNorientation, XmHORIZONTAL);
	else if (dir == gtVERT)
	    XtSetArg(args[0], XmNorientation, XmVERTICAL);
	XtSetArg(args[1], XmNsliderSize, 1);

	r->widget(XmCreateScrollBar(parent->container()->rep()->widget(),
				    (char*)name, args, sizeof(args)/sizeof(args[0])));

	add_callback(XmNvalueChangedCallback, this,
		     gtCallbackFunc(callback), client_data);
    }
    destroy_init();
}
 

gtScrollbarXm::~gtScrollbarXm()
{}


void gtScrollbarXm::set_limits(int min, int max)
{
    Arg args[2];
    XtSetArg(args[0], XmNminimum, min);
    XtSetArg(args[1], XmNmaximum, max);
    XtSetValues(r->widget(), args, 2);
}


int gtScrollbarXm::value()
{
    int val;

    Arg args[1];
    XtSetArg(args[0], XmNvalue, &val);
    XtGetValues(r->widget(), args, 1);

    return val;
}


void gtScrollbarXm::value(int v)
{
    Arg args[1];
    XtSetArg(args[0], XmNvalue, v);
    XtSetValues(r->widget(), args, 1);
}


void gtScrollbarXm::add_CB(gtScrollCB callback, void* client_data, gtReason rsn)
{ 
    gtReasonXm rea(rsn);
    add_callback(rea.reason_cb(), this, gtCallbackFunc(callback), client_data);
}


void gtScrollbarXm::slider_size(int s)
{
    Arg	args[1];
    XtSetArg(args[0], XmNsliderSize, s);
    XtSetValues(r->widget(), args, 1);
}

    
void gtScrollbarXm::arrows(int flag)
{
    Arg args[1];
    XtSetArg(args[0], XmNshowArrows, flag);
    XtSetValues(r->widget(), args, 1);
}


/*
   START-LOG-------------------------------------------

   $Log: gtScrollbarXm.h.C  $
   Revision 1.2 2000/12/18 17:08:49EST ktrans 
   Bug track: 19513.  Specify non-zero size when building a scrollbar
Revision 1.2.1.4  1993/06/02  20:28:52  glenn
Remove Initialize macros.  Remove genError.h.

Revision 1.2.1.3  1993/01/26  04:56:58  glenn
Use gtCallbackMgrXm.
Add dtor.

Revision 1.2.1.2  1992/10/09  19:03:47  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
