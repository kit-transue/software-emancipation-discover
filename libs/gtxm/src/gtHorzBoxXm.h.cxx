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
// gtHorzBoxXm
//------------------------------------------
// synopsis:
// Implentation of member functions for the HorzBox class (gtHorzBoxXm)
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtHorzBoxXm.h>

#include <Xm/RowColumn.h>


// FUNCTION DEFINITIONS

gtHorzBox* gtHorzBox::create(gtBase* parent, const char* name)
{
    return new gtHorzBoxXm(parent, name);
}


gtHorzBoxXm::gtHorzBoxXm(gtBase* parent, const char* name)
{
    if(parent)
    {
	Arg args[2];
	XtSetArg(args[0], XmNorientation, XmHORIZONTAL);
	XtSetArg(args[1], XmNisAligned, FALSE);
	r->widget(XmCreateRowColumn(parent->container()->rep()->widget(),
				    (char*)name, args, 2));
    }
    destroy_init();
}


void gtHorzBoxXm::margins(int h, int v)
{
    Arg args[2];
    XtSetArg(args[0], XmNmarginWidth, horzPixels(h));
    XtSetArg(args[1], XmNmarginHeight, vertPixels(v));
    XtSetValues(r->widget(), args, 2);
}


void gtHorzBoxXm::packing(gtPacking p)
{
    Arg args[1];
    XtSetArg(args[0], XmNpacking, p);
    XtSetValues(r->widget(), args, 1);
}


void gtHorzBoxXm::rows(int rw)
{
    Arg args[1];
    XtSetArg(args[0], XmNnumColumns, rw);
    XtSetValues(r->widget(), args, 1);
}


/*
   START-LOG-------------------------------------------

   $Log: gtHorzBoxXm.h.C  $
   Revision 1.1 1993/01/25 23:40:55EST builder 
   made from unix file
Revision 1.2.1.5  1993/01/26  04:32:20  glenn
Check parent in ctor.

Revision 1.2.1.4  1993/01/22  20:40:27  glenn
Set XmNisAligned to FALSE.

Revision 1.2.1.3  1992/12/30  19:24:16  glenn
Add margins, packing, rows.

Revision 1.2.1.2  1992/10/09  19:03:33  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
