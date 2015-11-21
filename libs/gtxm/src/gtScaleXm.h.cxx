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
// gtScaleXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtLabel class in OSF Motif
//------------------------------------------

// INCLUDE FILES

//#include <malloc.h>

#include <gtBaseXm.h>
#include <gtScaleXm.h>

#include <Xm/Scale.h>


// FUNCTION DEFINITIONS

gtScale* gtScale::create(gtBase *parent, const char* name, const char *contents,
			 gtDirection dir, int minval, int maxval)
{
    return new gtScaleXm(parent, name, contents, dir, minval, maxval);
}


gtScale* gtScale::create(gtBase *parent, const char* name, const char *contents,
			 gtDirection dir)
{
    return new gtScaleXm(parent, name, contents, dir);
}


gtScaleXm::gtScaleXm(gtBase *parent, const char *name, const char *contents,
		     gtDirection dir, int minval, int maxval)
{
    if(parent)
    {
	XmString st = XmStringCreateSimple((char*)gettext(contents));

	Arg args[4];
	XtSetArg(args[0], XmNtitleString, st);
	XtSetArg(args[1], XmNminimum, minval);
	XtSetArg(args[2], XmNmaximum, maxval);
	if (dir == gtHORZ)
	    XtSetArg(args[3], XmNorientation, XmHORIZONTAL);
	else
	    XtSetArg(args[3], XmNorientation, XmVERTICAL);

	r->widget(XmCreateScale(parent->container()->rep()->widget(),
				(char*)name, args, 4));

	XmStringFree(st);
    }
    destroy_init();
}


gtScaleXm::gtScaleXm(gtBase *parent, const char *name, const char *contents,
		     gtDirection dir)
{
    XmString st = XmStringCreateSimple((char*)gettext(contents));

    Arg args[2];
    XtSetArg(args[0], XmNtitleString, st);
    if (dir == gtHORZ)
	XtSetArg(args[1], XmNorientation, XmHORIZONTAL);
    else
	XtSetArg(args[1], XmNorientation, XmVERTICAL);

    r->widget(XmCreateScale(parent->container()->rep()->widget(),
			    (char*)name, args, 2));

    XmStringFree(st);
    destroy_init();
}


gtScaleXm::~gtScaleXm()
{}


gtBase *gtScaleXm::container()
{
    return NULL;
}


void gtScaleXm::add_callback(
    gtReason rea, gtScaleCB callback, void* client_data)
{
    gtReasonXm rxm(rea);

    gtCallbackMgrXm::add_callback(
	rxm.reason_cb(), this, gtCallbackFunc(callback), client_data);
}
 

int gtScaleXm::value()
{
    int v;
    XmScaleGetValue(r->widget(), &v);
    return v;
}


void gtScaleXm::value(int v)
{
    XmScaleSetValue(r->widget(), v);
}


void gtScaleXm::decimal_points(int p)
{
    Arg	args[1];
    XtSetArg(args[0], XmNdecimalPoints, p);
    XtSetValues(r->widget(), args, 1);
}


void gtScaleXm::show_value(int flag)
{
    Arg	args[1];
    XtSetArg(args[0], XmNshowValue, flag);
    XtSetValues(r->widget(), args, 1);
}


/*
   START-LOG-------------------------------------------

   $Log: gtScaleXm.h.C  $
   Revision 1.1 1993/01/25 23:53:12EST builder 
   made from unix file
Revision 1.2.1.3  1993/01/26  04:55:21  glenn
Check parent in ctor.
Use gtCallbackMgrXm.
Add dtor.

Revision 1.2.1.2  1992/10/09  19:03:45  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
