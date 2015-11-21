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
// gtLabelXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtLabel class in OSF Motif
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtLabelXm.h>

#include <Xm/Label.h>


// FUNCTION DEFINITIONS

gtLabel* gtLabel::create(gtBase *parent, const char* name, const char *contents)
{
    return new gtLabelXm(parent, name, contents);
}

gtLabelXm::gtLabelXm(gtBase *parent, const char* name, const char* contents)
{
    if(parent)
    {
	XmString st = NULL;
	Arg args[1];
	int n = 0;

	if(contents)
	{
	    st = XmStringCreateLtoR((char*)gettext(contents),
				    XmSTRING_DEFAULT_CHARSET);
	    XtSetArg(args[0], XmNlabelString, st); n++;
	}
	r->widget(
	    XtCreateManagedWidget(name, xmLabelWidgetClass,
				  parent->container()->rep()->widget(),
				  args, n));
	if(st)
	    XmStringFree(st);
    }
    destroy_init();
}    


gtBase *gtLabelXm::container()
{
    return NULL;
}


void gtLabelXm::text(const char* tx)
{
    if(tx == NULL)
	tx = "";

    XmString st = XmStringCreateLtoR(
	(char *)gettext(tx), XmSTRING_DEFAULT_CHARSET);
    Arg args[1];
    XtSetArg(args[0], XmNlabelString, st);
    XtSetValues(r->widget(), args, 1);

    XmStringFree(st);
}


void gtLabelXm::alignment(gtAlign a)
{
    Arg		args[1];

    if (a == gtBeginning)
	XtSetArg(args[0], XmNalignment, XmALIGNMENT_BEGINNING);
    if (a == gtCenter)
	XtSetArg(args[0], XmNalignment, XmALIGNMENT_CENTER);
    if (a == gtEnd)
	XtSetArg(args[0], XmNalignment, XmALIGNMENT_END);
    XtSetValues(r->widget(), args, 1);
}


/*
   START-LOG-------------------------------------------

   $Log: gtLabelXm.h.C  $
   Revision 1.1 1993/01/25 23:40:55EST builder 
   made from unix file
Revision 1.2.1.3  1993/01/26  04:32:20  glenn
Check parent in ctor.

Revision 1.2.1.2  1992/10/09  19:03:33  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
