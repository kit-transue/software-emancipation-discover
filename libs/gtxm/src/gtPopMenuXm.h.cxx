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
// gtPopMenuXm.h.C 
// ------------------------------------------ 
// synopsis: 
// Member function source code for gtPopupMenu class in OSF Motif 
// 
//------------------------------------------ 

// INCLUDE FILES

#include <stdarg.h>
#include <psetmem.h>
#include <machdep.h>   // for OSdependent stuff (OSapi_free OSapi_realloc)

#include <gtSepar.h>
#include <gtLabel.h>
#include <gtPushButton.h>
#include <gtTogB.h>
#include <gtCascadeB.h>

#include <gtBaseXm.h>
#include <gtPopMenuXm.h>

#include <Xm/RowColumn.h>
#include <Xm/MenuShell.h>

// CLASS STATICS

Cardinal gtPopupMenuXm::ins_loc;

// FUNCTION DEFINITIONS

gtPopupMenu* gtPopupMenu::create(gtBase *parent, const char *name ...)
{
    va_list ap;
    va_start(ap, name);
    gtPopupMenuXm* ptr = new gtPopupMenuXm(parent, name, ap);
    va_end(ap);

    return ptr;
}

//
// handle_right_button
//
// Positions and pops up menu when button 3 is pressed.
//

static void handle_right_button(Widget, XtPointer popup, XEvent* e, Boolean*)
{
    if(e->xany.type == ButtonPress  &&  e->xbutton.button == 3)
    {
	XmMenuPosition(Widget(popup), &e->xbutton);
	XtManageChild(Widget(popup));
    }
}


Cardinal gtPopupMenuXm::insert_at_loc(Widget)
{
    return ins_loc;
}


gtPopupMenuXm::gtPopupMenuXm(gtBase* parent, const char* name, va_list& ap)
: num_entries(0),
  entries((gtPrimitive**)psetmalloc(sizeof(gtPrimitive*)))
{
    if(parent)
    {
	r->widget(XmCreatePopupMenu(parent->container()->rep()->widget(),
				    (char*)name, NULL, 0));

	Arg args[1];
	XtSetArg(args[0], XmNinsertPosition, insert_at_loc);
	XtSetValues(r->widget(), args, 1);

	var_insert(0, ap);

	XtAddEventHandler(parent->container()->rep()->widget(),
			  ButtonPressMask, False, handle_right_button,
			  XtPointer(r->widget()));
    }
    destroy_init();
}


gtPopupMenuXm::~gtPopupMenuXm()
{
    if(entries)
	OSapi_free(entries);
}


void gtPopupMenuXm::var_insert(int loc, va_list& ap)
{
    ins_loc = loc;		// ins_loc is class static.
    if(ins_loc < 0  ||  ins_loc > num_entries)
	ins_loc = num_entries;

    gtMenuChoice entry;
    while(entry = va_arg(ap, gtMenuChoice))
    {
	gtPrimitive* prim = NULL;
	const char* name = va_arg(ap, const char*);
	const char* label = NULL;
	void* callback_data = NULL;
	gtPushCB callback = NULL;

	switch(entry)
	{
	  case gtMenuSeparator:
	    if (name == NULL)
		name = "separator";
	    prim = gtSeparator::create(this, name, gtHORZ);
	    break;

	  case gtMenuLabel:
	    if (name == NULL)
		name = "label";
	    label = va_arg(ap, const char*);
	    prim = gtLabel::create(this, name, label);
	    break;

	  default:
	    label = va_arg(ap, const char*);
	    callback_data = va_arg(ap, void*);
	    callback = va_arg(ap, gtPushCB);

	    switch(entry)
	    {
	      case gtMenuStandard:
		if(name == NULL)
		    name = "pushbutton";
		prim = gtPushButton::create(
		    this, name, label, gtPushCB(callback), callback_data);
		break;

	      case gtMenuCheck:
		if(name == NULL)
		    name = "togglebutton";
		prim = gtToggleButton::create(
		    this, name, label, gtToggleCB(callback), callback_data);
		break;

	      case gtMenuCascade:
		if(name == NULL)
		    name = "cascadebutton";
		prim = gtCascadeButton::create(
		    this, name, label, gtCascadeCB(callback), callback_data);
		break;

	      default:		// Unknown entry type -> bail out.
		return;
	    }
	    break;
	}

	prim->manage();

	entries = (gtPrimitive**)psetrealloc((char *)entries,
		     ++num_entries * sizeof(gtPrimitive*));
	for(int i = num_entries - 1; i > ins_loc; --i)
	    entries[i] = entries[i - 1];

	entries[ins_loc++] = prim;
    }
}


void gtPopupMenuXm::move_to(gtEventPtr eve)
{
    XmMenuPosition(r->widget(), &(eve->xbutton));
}


void gtPopupMenuXm::insert_entries(int loc ...)
{
    va_list ap;
    va_start(ap, loc);
    var_insert(loc, ap);
    va_end(ap);
}


void gtPopupMenuXm::remove_entry(int loc)
{
    delete entries[loc];

    num_entries--;
    for(int n = loc; n < num_entries - 1; n++)
	entries[n] = entries[n+1];
}


gtPrimitive* gtPopupMenuXm::entry(int n)
{
    return entries[n];
}


void gtPopupMenuXm::map_callback(gtPopupCB callback, void *client_data)
{
    add_callback(XmNmapCallback, this, gtCallbackFunc(callback), client_data);
}    


/*
   START-LOG-------------------------------------------

   $Log: gtPopMenuXm.h.C  $
   Revision 1.5 2000/07/07 08:10:42EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.8  1994/04/13  14:17:17  builder
Port - use aset_stdarg first

Revision 1.2.1.7  1994/02/16  18:08:16  kws
Port

Revision 1.2.1.6  1994/01/13  02:46:20  kws
Use psetmalloc

Revision 1.2.1.5  1993/10/07  20:44:39  kws
Port

Revision 1.2.1.4  1993/06/02  20:18:08  glenn
Remove Initialize and Return macros.  Remove genError.h.

Revision 1.2.1.3  1993/01/26  04:51:23  glenn
Use gtCallbackMgrXm.
Add dtor.

Revision 1.2.1.2  1992/10/09  19:03:41  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
