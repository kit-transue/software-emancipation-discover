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
// gtToggleBoxXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtToggleBox class in OSF Motif
//------------------------------------------

// INCLUDE FILES

#include <stdarg.h>
#include <psetmem.h>
#include <machdep.h>    // for OSapi_free and OSapi_realloc

#include <gtVertBox.h>
#include <gtFrame.h>
#include <gtLabel.h>
#include <gtTogB.h>

#include <gtBaseXm.h>
#include <gtToggleBoxXm.h>

#include <Xm/RowColumn.h>


// FUNCTION DEFINITIONS

gtToggleBox* gtToggleBox::create(
    gtBase* parent, const char *name, const char *title ...)
{
    va_list ap;
    va_start(ap, title);
    gtToggleBoxXm* ptr = new gtToggleBoxXm(parent, name, title, &ap);
    va_end(ap);

    return ptr;
}


gtToggleBoxXm::gtToggleBoxXm(
    gtBase *parent, const char *name, const char *title, va_list *pap)
{
    delete r;
    gtFrame* button_frame = NULL;
    if(title)
    {
	gtVertBox* pane = gtVertBox::create(parent, name);
	attach_rep(pane->rep());

	gtLabel* box_title = gtLabel::create(pane, name, title);
	box_title->manage();

	button_frame = gtFrame::create(pane, "button_frame");
	button_frame->manage();
    }
    else
    {
	button_frame = gtFrame::create(parent, "button_frame");
	attach_rep(button_frame->rep());
    }

    button_group = gtVertBox::create(button_frame, "button_group", gtBeginning);
    button_group->manage();

    Arg args[1];
    XtSetArg(args[0], XmNindicatorType, XmN_OF_MANY);

    num_buttons = 0;
    buttons = (gtToggleButton**)psetmalloc(sizeof(gtToggleButton));

    const char* btitle;
    while(btitle = va_arg((*pap), const char *))
    {
	const char* buttname = va_arg((*pap), const char*);
	gtToggleCB  callback = va_arg((*pap), gtToggleCB);
	void *client_data = va_arg((*pap), void*);

	if (buttname == NULL)
	    buttname = "toggle_button";

	buttons = (gtToggleButton **)psetrealloc((char *)buttons,
           (num_buttons + 1) * sizeof(gtToggleButton*));

	buttons[num_buttons] = gtToggleButton::create(
	    button_group, buttname, btitle, callback, client_data);

	XtSetValues(buttons[num_buttons]->rep()->widget(), args, 1);
	buttons[num_buttons++]->manage();
    }
}


gtToggleBoxXm::~gtToggleBoxXm()
{
    if(buttons)
	OSapi_free(buttons);
}


void gtToggleBoxXm::num_columns(int n)
{
    Arg		args[2];

    XtSetArg(args[0], XmNnumColumns, n);
    XtSetArg(args[1], XmNpacking, XmPACK_COLUMN);
    XtSetValues(button_group->container()->rep()->widget(), args, 2);
}

gtBase *gtToggleBoxXm::container()
{
    return button_group;
}

int gtToggleBoxXm::item_set(int num)
{
    int ans;
    ans = buttons[num]->set();
    return ans;
}

void gtToggleBoxXm::item_set(int num, int value, int flag)
{
    buttons[num]->set(value, flag);
}
    
gtToggleButton *gtToggleBoxXm::button(const char *nm)
{
    int i;	
    for (i=0; i<num_buttons; i++) {
	if (strcmp(buttons[i]->name(), nm) == 0)
	    return buttons[i];
    }
    return NULL;
}

/*
   START-LOG-------------------------------------------

   $Log: gtToggleBoxXm.h.C  $
   Revision 1.4 2000/07/07 08:10:52EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.8  1994/04/13  14:17:17  builder
Port - use aset_stdarg first

Revision 1.2.1.7  1994/01/13  02:46:20  kws
Use psetmalloc

Revision 1.2.1.6  1993/10/07  20:44:39  kws
Port

Revision 1.2.1.5  1993/04/27  21:50:57  glenn
Delete the initial gtRep instead of using release_rep.

Revision 1.2.1.4  1993/04/23  17:32:37  davea
bug 3482 - added return to button()

Revision 1.2.1.3  1993/01/26  05:01:51  glenn
Add dtor.
Remove unnecessary members.

Revision 1.2.1.2  1992/10/09  19:03:58  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
