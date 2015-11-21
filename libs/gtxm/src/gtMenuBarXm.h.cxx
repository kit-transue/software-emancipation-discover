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
// gtMenuBarXm
//------------------------------------------
// synopsis:
// Implentation of member functions for the MenuBar class (gtMenuBarXm)
//------------------------------------------

// INCLUDE FILES

#include <gtCascadeB.h>
#include <gtBaseXm.h>
#include <gtMenuBarXm.h>

#include <Xm/RowColumn.h>


// FUNCTION DEFINITIONS

gtMenuBar* gtMenuBar::create(gtBase *parent, const char* name)
{
    return new gtMenuBarXm(parent, name);
}

gtMenuBarXm::gtMenuBarXm(gtBase *parent, const char *name)
{
    if(parent)
    {
	r->widget(XmCreateMenuBar(parent->container()->rep()->widget(),
				  (char *)name,NULL,0));
    }
    destroy_init();
}

void gtMenuBarXm::set_help(gtCascadeButton *casc)
{
    Arg args[1];
    XtSetArg(args[0], XmNmenuHelpWidget, casc->rep()->widget());
    XtSetValues(r->widget(), args, 1);
}


/*
   START-LOG-------------------------------------------

   $Log: gtMenuBarXm.h.C  $
   Revision 1.1 1993/01/25 23:40:56EST builder 
   made from unix file
Revision 1.2.1.3  1993/01/26  04:32:20  glenn
Check parent in ctor.

Revision 1.2.1.2  1992/10/09  19:03:36  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
