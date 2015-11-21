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
// gtPDMenuXm.h.C 
// ------------------------------------------ 
// synopsis: 
// Member function source code for gtPulldownMenu class in OSF Motif 
//------------------------------------------

// INCLUDE FILES

#include <stdarg.h>

#include <psetmem.h>
#include <machdep.h>  // For OSapi_free and OSapi_realloc

#include <gtNoParent.h>
#include <gtSepar.h>
#include <gtLabel.h>
#include <gtPushButton.h>
#include <gtTogB.h>
#include <gtCascadeB.h>

#include <gtBaseXm.h>
#include <gtPDMenuXm.h>

#include <Xm/RowColumn.h>
#include <Xm/MenuShell.h>

// CLASS STATICS
 
Cardinal gtPulldownMenuXm::ins_loc;

// FUNCTION DEFINITIONS

gtPulldownMenu* gtPulldownMenu::create(gtBase* parent, const char* name ...)
{
    va_list ap;
    va_start(ap, name);
    gtPulldownMenuXm* ptr = new gtPulldownMenuXm(parent, name, &ap);
    va_end(ap);

    return ptr;
}


gtPulldownMenuXm::gtPulldownMenuXm(
    gtBase* parent, const char* name, va_list* pap)
: num_entries(0),
  entries((gtPrimitive**)psetmalloc(sizeof(gtPrimitive*))),
  num_cascades(0),
  cascades((gtCascadeButton**)psetmalloc(sizeof(gtCascadeButton*)))
{
    if(parent)
    {
	if(name == NULL)
	    name = "pulldown_menu";

	r->widget(XmCreatePulldownMenu(parent->container()->rep()->widget(),
				       (char*)name, NULL, 0));
	var_insert(0, pap);
    }
    destroy_init();
}


gtPulldownMenuXm::~gtPulldownMenuXm()
{
    if(entries)
	OSapi_free(entries);
    if(cascades)
	OSapi_free(cascades);
}


void gtPulldownMenuXm::var_insert(int loc, va_list* pap)
{
    Arg args[2];
    
    ins_loc = (loc < 0) ? num_entries : loc;

    XtSetArg(args[0], XmNinsertPosition, insert_at_loc);
    XtSetValues(r->widget(), args, 1);

    gtMenuChoice entry;
    while(entry = va_arg((*pap), gtMenuChoice))
    {
	const char *buttname = va_arg((*pap), const char*);
	entries = (gtPrimitive**)psetrealloc((char *)
	    entries, (num_entries + 1) * sizeof(gtPrimitive*));

	if(entry == gtMenuSeparator)
	{
	    if (buttname == NULL)
		buttname = "separator";

	    entries[num_entries] = gtSeparator::create(this, buttname, gtHORZ);
	}
	else if (entry == gtMenuLabel)
	{
	    const char* contents = va_arg((*pap), const char*);

	    if (buttname == NULL)
		buttname = "label";

	    entries[num_entries] = gtLabel::create(this, buttname, contents);
	}
	else
	{
	    const char* contents = va_arg((*pap), const char*);
	    void* client_data = va_arg((*pap), void*);

	    if (buttname == NULL)
		buttname = contents;

	    if (entry == gtMenuStandard)
	    {
		gtPushCB callback = va_arg((*pap), gtPushCB);
		entries[num_entries] = gtPushButton::create(
		    this, buttname, contents, callback, client_data);

		XtSetArg(args[0], XmNmarginTop, 0);
		XtSetArg(args[1], XmNmarginBottom, 0);
		XtSetValues(entries[num_entries]->rep()->widget(), args, 2);
	    }
	    else if (entry == gtMenuCheck)
	    {
		gtToggleCB callback = va_arg((*pap), gtToggleCB);

		entries[num_entries] = gtToggleButton::create(
		    this, buttname, contents, callback, client_data);
	    }
	    else if (entry == gtMenuRadio)
	    {
		gtToggleCB callback = va_arg((*pap), gtToggleCB);

		entries[num_entries] = gtToggleButton::create(
		    this, buttname, contents, callback, client_data);

		XtSetArg(args[0], XmNindicatorType, XmONE_OF_MANY);
		XtSetValues(entries[num_entries]->rep()->widget(), args, 1);
	    }
	    else
	    {
		gtCascadeCB callback = va_arg((*pap), gtCascadeCB);

		cascades = (gtCascadeButton**)psetrealloc((char *)
		    cascades, (num_cascades + 1) * sizeof(gtCascadeButton*));
		cascades[num_cascades] = gtCascadeButton::create(
		    this, buttname, contents, callback, client_data);

		entries[num_entries] = cascades[num_cascades++];
	    }
	}
	entries[num_entries++]->manage();
	ins_loc++;
    }
}


Cardinal gtPulldownMenuXm::insert_at_loc(Widget)
{
    return ins_loc;
}


void gtPulldownMenuXm::move_to(gtEventPtr eve)
{
    XmMenuPosition(r->widget(), &(eve->xbutton));
}


void gtPulldownMenuXm::_filler1_()
{}


void gtPulldownMenuXm::_filler2_()
{}


void gtPulldownMenuXm::insert_entries(int loc ...)
{
    va_list ap;
    va_start(ap, loc);
    var_insert(loc, &ap);
    va_end(ap);
}


void gtPulldownMenuXm::remove_entry(int loc)
{
    delete entries[loc];
    for (int n = loc; n < num_entries - 1; n++)
	entries[n] = entries[n + 1];
    num_entries--;
}


gtPrimitive* gtPulldownMenuXm::entry(int n)
{
    return entries[n];
}


gtCascadeButton* gtPulldownMenuXm::cascade(int n)
{
    return cascades[n];
}


gtPrimitive* gtPulldownMenuXm::button(const char* name)
{
    for(int n = 0; n < num_entries; n++)
	if(strcmp(name, entries[n]->name()) == 0)
	    return entries[n];
    return NULL;
}


void gtPulldownMenuXm::map_callback(gtPulldownCB callback, void *client_data)
{
    add_callback(XmNmapCallback, this, gtCallbackFunc(callback), client_data);
}    


/*
   START-LOG-------------------------------------------

   $Log: gtPDMenuXm.h.C  $
   Revision 1.5 2000/07/07 08:10:40EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.9  1994/04/13  14:17:17  builder
Port - use aset_stdarg first

Revision 1.2.1.8  1994/02/16  18:08:14  kws
Port

Revision 1.2.1.7  1994/01/13  02:46:20  kws
Use psetmalloc

Revision 1.2.1.6  1993/10/07  20:44:39  kws
Port

Revision 1.2.1.5  1993/06/02  20:18:08  glenn
Remove Initialize and Return macros.  Remove genError.h.

Revision 1.2.1.4  1993/02/25  03:59:33  glenn
Remove unnecessary call to XtDestroyWidget in remove_entry that was
causing Purify errors.

Revision 1.2.1.3  1993/01/26  04:49:45  glenn
Use gtCallbackMgrXm.
Add dtor.

Revision 1.2.1.2  1992/10/09  19:03:39  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
