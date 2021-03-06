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
// gtSeparXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtSeparator class in OSF Motif
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtSeparXm.h>

#include <Xm/Separator.h>


// FUNCTION DEFINITIONS

gtSeparator* gtSeparator::create(gtBase *parent, const char* name, gtDirection dir)
{
    return new gtSeparatorXm(parent, name, dir);
}

gtSeparatorXm::gtSeparatorXm(gtBase *parent, const char *name, gtDirection dir)
{
    if(parent)
    {
	Arg args[1];

	if (dir == gtHORZ)
	    XtSetArg(args[0], XmNorientation, XmHORIZONTAL);
	else if (dir == gtVERT)
	    XtSetArg(args[0], XmNorientation, XmVERTICAL);

	r->widget(XmCreateSeparator(parent->container()->rep()->widget(),
				    (char *)name, args, 1));
    }
    destroy_init();
}

gtBase *gtSeparatorXm::container()
{
    return NULL;
}

void gtSeparatorXm::set_style(gtSeparatorStyle sty)
{
    Arg args[1];

    if (sty == gtNONE) {
	XtSetArg(args[0], XmNseparatorType, XmNO_LINE);
    } else if (sty == gtSINGLE) {
	XtSetArg(args[0], XmNseparatorType, XmSINGLE_LINE);
    } else if (sty == gtDOUBLE) {
	XtSetArg(args[0], XmNseparatorType, XmDOUBLE_LINE);
    } else if (sty == gtSINGLE_DASHED) {
	XtSetArg(args[0], XmNseparatorType, XmSINGLE_DASHED_LINE);
    } else if (sty == gtDOUBLE_DASHED) {
	XtSetArg(args[0], XmNseparatorType, XmDOUBLE_DASHED_LINE);
    } else if (sty == gtSHADOW_IN) {
	XtSetArg(args[0], XmNseparatorType, XmSHADOW_ETCHED_IN);
    } else if (sty == gtSHADOW_OUT) {
	XtSetArg(args[0], XmNseparatorType, XmSHADOW_ETCHED_OUT);
    }
    
    XtSetValues(r->widget(), args, 1);
}


/*
   START-LOG-------------------------------------------

   $Log: gtSeparXm.h.C  $
   Revision 1.1 1993/01/25 23:55:55EST builder 
   made from unix file
Revision 1.2.1.3  1993/01/26  04:57:59  glenn
Remove construct member.

Revision 1.2.1.2  1992/10/09  19:03:51  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
