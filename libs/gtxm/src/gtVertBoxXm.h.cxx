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
// gtVertBoxXm
//------------------------------------------
// synopsis:
// Implentation of member functions for the VertBox class (gtVertBoxXm)
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtVertBoxXm.h>

#include <Xm/RowColumn.h>


// FUNCTION DEFINITIONS

gtVertBox* gtVertBox::create(gtBase* parent, const char* name)
{
    return new gtVertBoxXm(parent, name, gtAlign(-1));
}

gtVertBox* gtVertBox::create(gtBase* parent, const char* name, gtAlign a)
{
    return new gtVertBoxXm(parent, name, a);
}

gtVertBoxXm::gtVertBoxXm(gtBase* parent, const char* name, gtAlign a)
{
    if(parent)
    {
	Arg args[2];

	int alignment = -1;
	switch(a)
	{
	  case gtBeginning: alignment = XmALIGNMENT_BEGINNING; break;
	  case gtCenter:    alignment = XmALIGNMENT_CENTER;    break;
	  case gtEnd:       alignment = XmALIGNMENT_END;       break;
	}
	if(alignment >= 0)
	    XtSetArg(args[0], XmNentryAlignment, alignment);
	else
	    XtSetArg(args[0], XmNisAligned, FALSE);
	
	XtSetArg(args[1], XmNorientation, XmVERTICAL);

	r->widget(XmCreateRowColumn(parent->container()->rep()->widget(),
				    (char*)name, args, 2));
    }
    destroy_init();
}

void gtVertBoxXm::margins(int h, int v)
{
    Arg args[2];
    XtSetArg(args[0], XmNmarginWidth, horzPixels(h));
    XtSetArg(args[1], XmNmarginHeight, vertPixels(v));
    XtSetValues(r->widget(), args, 2);
}

void gtVertBoxXm::packing(gtPacking p)
{
    Arg args[1];
    XtSetArg(args[0], XmNpacking, p);
    XtSetValues(r->widget(), args, 1);
}

void gtVertBoxXm::columns(int c)
{
    Arg args[1];
    XtSetArg(args[0], XmNnumColumns, c);
    XtSetValues(r->widget(), args, 1);
}


/*
   START-LOG-------------------------------------------

   $Log: gtVertBoxXm.h.C  $
   Revision 1.1 1993/01/26 00:01:48EST builder 
   made from unix file
Revision 1.2.1.5  1993/01/26  05:04:31  glenn
Add create with gtAlign.
Add gtAlign to ctor.

Revision 1.2.1.4  1993/01/22  20:40:27  glenn
Set XmNisAligned to FALSE.

Revision 1.2.1.3  1992/12/30  19:24:16  glenn
Add margins, packing, columns.

Revision 1.2.1.2  1992/10/09  19:04:08  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
