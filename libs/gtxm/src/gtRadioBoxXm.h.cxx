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
// gtRadioBoxXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtRadioBox class in OSF Motif
//
// description:

// INCLUDE FILES

#include <stdarg.h>
#define _cLibraryFunctions_h_
#include <psetmem.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <machdep.h>   // for OSapi_free and OSapi_realloc

#include <gtNoParent.h>
#include <gtVertBox.h>
#include <gtFrame.h>
#include <gtLabel.h>
#include <gtTogB.h>

#include <gtBaseXm.h>
#include <gtRadioBoxXm.h>

#include <Xm/RowColumn.h>


// FUNCTION DEFINITIONS

gtRadioBox* gtRadioBox::create(
    gtBase* parent, const char* name, const char* title ...)
{
    va_list ap;
    va_start(ap, title);
    gtRadioBoxXm* ptr = new gtRadioBoxXm(parent, name, title, &ap);
    va_end(ap);

    return ptr;
}


gtRadioBoxXm::gtRadioBoxXm(
    gtBase* parent, const char* name, const char* title, va_list* pap)
{
    delete r;
    gtFrame* button_frame = NULL;
    if(title)
    {
	gtVertBox* pane = gtVertBox::create(parent, name);
	attach_rep(pane->rep());

	gtLabel* box_title = gtLabel::create(pane, "title", title);
	box_title->manage();

	button_frame = gtFrame::create(pane, "frame");
	button_frame->manage();
    }
    else
    {
	button_frame = gtFrame::create(parent, name);
	attach_rep(button_frame->rep());
    }

    Widget rbox = XmCreateRadioBox(button_frame->container()->rep()->widget(),
				   (char*)name, NULL, 0);
    xm_radio_box = gtNoParent::create();
    xm_radio_box->attach_rep(new gtRep(rbox));
    xm_radio_box->manage();

    Arg args[1];
    XtSetArg(args[0], XmNindicatorType, XmONE_OF_MANY);

    buttons = (gtToggleButton**)psetmalloc(sizeof(gtToggleButton*));
    num_buttons = 0;

    const char* btitle;
    while(btitle = va_arg((*pap), const char*))
    {
	const char* buttname = va_arg((*pap), const char*);
	gtToggleCB  callback = va_arg((*pap), gtToggleCB);
	void*    client_data = va_arg((*pap), void*);

	if (buttname == NULL)
	    buttname = "radio_button";

	buttons = (gtToggleButton**)psetrealloc((char *)buttons, 
            (num_buttons + 1) * sizeof(gtToggleButton*));

	buttons[num_buttons] = gtToggleButton::create(
	    this, buttname, btitle, callback, client_data);

	XtSetValues(buttons[num_buttons]->rep()->widget(), args, 1);
	buttons[num_buttons++]->manage();
    }
}


gtRadioBoxXm::~gtRadioBoxXm()
{
    if(buttons)
	OSapi_free(buttons);
}


gtBase *gtRadioBoxXm::container()
{
    return xm_radio_box;
}


void gtRadioBoxXm::num_columns(int n)
{
    Arg args[2];
    XtSetArg(args[0], XmNnumColumns, n);
    XtSetArg(args[1], XmNpacking, XmPACK_COLUMN);
    XtSetValues(xm_radio_box->rep()->widget(), args, 2);
}


int gtRadioBoxXm::item_set()
{
    for(int i = 0; i < num_buttons; i++)
	if(buttons[i]->set())
	    return i + 1;

    return 0;
}


void gtRadioBoxXm::item_set(int num, int value, int flag)
{
    buttons[num]->set(value, flag);
}

    
gtToggleButton* gtRadioBoxXm::button(const char *nm)
{
    for(int i = 0; i < num_buttons; i++)
	if(strcmp(buttons[i]->name(), nm) == 0)
	    return buttons[i];

    return NULL;
}
	

/*
   START-LOG-------------------------------------------

   $Log: gtRadioBoxXm.h.C  $
   Revision 1.6 2000/07/10 23:05:34EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.8  1994/04/13  14:17:17  builder
Port - use aset_stdarg first

Revision 1.2.1.7  1994/01/13  02:46:20  kws
Use psetmalloc

Revision 1.2.1.6  1993/10/07  20:44:39  kws
Port

Revision 1.2.1.5  1993/04/27  21:50:57  glenn
Delete the initial gtRep instead of using release_rep.

Revision 1.2.1.4  1993/03/04  00:15:38  jon
Fixed problem with too many attach_reps in constructor

Revision 1.2.1.3  1993/01/26  04:54:06  glenn
Remove unnecessary member variables.
Add dtor.

Revision 1.2.1.2  1992/10/09  19:03:44  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
