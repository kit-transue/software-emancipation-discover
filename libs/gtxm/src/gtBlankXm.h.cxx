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
// gtBlankXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtBlank class in OSF Motif
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtBlankXm.h>

#include <Xm/DrawingA.h>


// FUNCTION DEFINITIONS

gtBlank* gtBlank::create(gtBase* parent, const char* name,
			 int width, int height)
{
    return new gtBlankXm(parent, name, width, height);
}


gtBlankXm::gtBlankXm(gtBase* parent, const char* name, int width, int height)
{
    if(parent)
    {
	Arg args[2];
	int n = 0;

	if (width > 0)
	{
	    XtSetArg(args[n], XmNwidth, width); n++; 
	}
	if (height > 0)
	{
	    XtSetArg(args[n], XmNheight, height); n++; 
	}
	r->widget(XmCreateDrawingArea(parent->container()->rep()->widget(),
				      (char*)name, args, n));
    }
    destroy_init();
}    


gtBlankXm::~gtBlankXm()
{}


void gtBlankXm::resize_policy(gtResizeOption opt)
{
    int policy = -1;
    switch(opt)
    {
      case gtResizeNone:	policy = XmRESIZE_NONE;		break;
      case gtResizeGrow:	policy = XmRESIZE_GROW;		break;
      case gtResizeAny:		policy = XmRESIZE_ANY;		break;
    }
    if(policy >= 0)
    {
	Arg args[1];
	XtSetArg(args[0], XmNresizePolicy, policy);
	XtSetValues(r->widget(), args, 1);
    }
}


void gtBlankXm::set_background(const char* color)
{
    XtVaSetValues(
	r->widget(),
	XtVaTypedArg, XmNbackground, XmRString, color, strlen(color) + 1,
	NULL);
}


void gtBlankXm::clear_window()
{
    XClearWindow(XtDisplay(r->widget()), XtWindow(r->widget()));
}


void gtBlankXm::expose_callback(gtBlankCB callback, void *client_data)
{ 
    add_callback(XmNexposeCallback, this,
		 gtCallbackFunc(callback), client_data);
}


/*
   START-LOG-------------------------------------------

   $Log: gtBlankXm.h.C  $
   Revision 1.1 1993/01/25 23:40:53EST builder 
   made from unix file
Revision 1.2.1.3  1993/01/26  04:32:20  glenn
Use gtCallbackMgrXm.

Revision 1.2.1.2  1992/10/09  19:03:25  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
