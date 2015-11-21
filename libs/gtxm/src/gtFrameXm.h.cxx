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
// gtFrameXm
//------------------------------------------
// synopsis:
// Implentation of member functions for the Frame class (gtFrameXm)
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtFrameXm.h>

#include <Xm/Frame.h>


// FUNCTION DEFINITIONS

gtFrame* gtFrame::create(gtBase *parent, const char* name)
{
    return new gtFrameXm(parent, name);
}


gtFrame* gtFrame::create(
    gtBase *parent, const char *name, gtFrameStyle s, int t, int b)
{
    gtFrameXm* frame = new gtFrameXm(parent, name);
    frame->set_style(s);
    frame->thickness(t);
    frame->border(b);

    return frame;
}


gtFrameXm::gtFrameXm(gtBase *parent, const char *name)
{
    if(parent)
    {
	r->widget(XmCreateFrame(parent->container()->rep()->widget(),
				(char *)name, NULL, 0));
    }
    destroy_init();
}


void gtFrameXm::set_style(gtFrameStyle sty)
{
    int type = 0;
    switch(sty)
    {
      case gtETCHED_IN:		type = XmSHADOW_ETCHED_IN;	break;
      case gtETCHED_OUT:	type = XmSHADOW_ETCHED_OUT;	break;
      case gtIN:		type = XmSHADOW_IN;		break;
      case gtOUT:		type = XmSHADOW_OUT;		break;
    }
    if(type)
    {
	Arg args[1];
	XtSetArg(args[0], XmNshadowType, type);
	XtSetValues(r->widget(), args, 1);
    }
}


void gtFrameXm::highlight(int on)
{
    Arg args[1];
    if(on)
    {
	XtSetArg(args[0], XmNborderColor, "black");
    }
    else
    {
	Pixel pix;
	XtSetArg(args[0], XmNbackground, &pix);
	XtGetValues(r->widget(), args, 1);
	XtSetArg(args[0], XmNborderColor, pix);
    }
    XtSetValues(r->widget(), args, 1);
}


void gtFrameXm::thickness(int value)
{
    Arg args[1];
    XtSetArg(args[0], XmNshadowThickness, value);
    XtSetValues(r->widget(), args, 1);
}


/*
   START-LOG-------------------------------------------

   $Log: gtFrameXm.h.C  $
   Revision 1.1 1993/01/25 23:40:54EST builder 
   made from unix file
Revision 1.2.1.4  1993/01/26  04:32:20  glenn
Check parent in ctor.

Revision 1.2.1.3  1993/01/22  17:14:12  glenn
Add create function that takes style, thickness, and border.
Use switch statment when setting style.

Revision 1.2.1.2  1992/10/09  19:03:32  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
