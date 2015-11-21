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
// gtOptionMenuXm.h.C 
// ------------------------------------------ 
// synopsis: 
// Member function source code for gtOptionMenu class in OSF Motif 
// 
//------------------------------------------ 

// INCLUDE FILES

#include <stdarg.h>
#include <psetmem.h>
#include <machdep.h> // For OSapi_free and OSapi_realloc

#include <gtNoParent.h>
#include <gtPrim.h>
#include <gtSepar.h>
#include <gtLabel.h>
#include <gtPushButton.h>
#include <gtTogB.h>
#include <gtCascadeB.h>
#include <gtPDMenu.h>

#include <gtBaseXm.h>
#include <gtOptionMenuXm.h>

#include <Xm/RowColumn.h>
#include <Xm/MenuShell.h>

// CLASS STATICS
 
Cardinal gtOptionMenuXm::ins_loc;

// FUNCTION DEFINITIONS

gtOptionMenu* gtOptionMenu::create(
    gtBase *parent, const char *name, const char *title ...)
{
    va_list ap;
    va_start(ap, title);
    gtOptionMenuXm* ptr = new gtOptionMenuXm(parent, name, title, &ap);
    va_end(ap);

    return ptr;
}


gtOptionMenuXm::gtOptionMenuXm(
    gtBase* parent, const char* name, const char* title, va_list* pap)
: num_entries(0),
  entries((gtPrimitive**)psetmalloc(sizeof(gtPrimitive*))),
  num_cascades(0),
  cascades((gtCascadeButton**)psetmalloc(sizeof(gtCascadeButton*))),
  gt_casc(NULL),
  pulldown_menu(NULL)
{
    if(title == NULL)
	title = name;
    XmString menu_title = XmStringCreateSimple((char*)gettext((char *)title));

    Widget pdm = XmCreatePulldownMenu(parent->container()->rep()->widget(),
				      (char*)name, NULL, 0);
    pulldown_menu = gtPulldownMenu::create(NULL, NULL);
    pulldown_menu->release_rep();
    pulldown_menu->attach_rep(new gtRep(pdm));

    Arg args[2];
    XtSetArg(args[0], XmNlabelString, menu_title);
    XtSetArg(args[1], XmNsubMenuId, pdm);

    r->widget(XmCreateOptionMenu(parent->container()->rep()->widget(),
				 (char*)title, args, 2));
    XmStringFree(menu_title);

    gt_casc = gtLabel::create(parent, name, title);
    gt_casc->release_rep();
    gt_casc->attach_rep(r);

    var_insert(0, pap);

    XtManageChild(r->widget());

    destroy_init();
}


gtOptionMenuXm::~gtOptionMenuXm()
{
    if(entries)
	OSapi_free(entries);
    if(cascades)
	OSapi_free(cascades);
}


Cardinal gtOptionMenuXm::insert_at_loc(Widget)
{
    return gtOptionMenuXm::ins_loc;
}


void gtOptionMenuXm::var_insert(int loc, va_list *pap)
{
    Arg args[2];

    ins_loc = (loc < 0) ? num_entries : loc;

    //XtSetArg(args[0], XmNinsertPosition, insert_at_loc);
    //XtSetValues(pulldown_menu->rep()->widget(), args, 1);

    gtMenuChoice entry;
    while(entry = va_arg((*pap), gtMenuChoice))
    {
	const char* buttname = va_arg((*pap), const char*);
	entries = (gtPrimitive**)psetrealloc((char *)entries, 
					     (num_entries + 1) * sizeof(gtPrimitive *));

	if (entry == gtMenuSeparator)
	{
	    if (buttname == NULL)
		buttname = "separator";
	    entries[num_entries] = gtSeparator::create(this, buttname, gtHORZ);
	}
	else if (entry == gtMenuLabel)
	{
	    const char* contents = va_arg((*pap), const char*);
	    if(buttname == NULL)
		buttname = "label";
	    entries[num_entries] = gtLabel::create(this, buttname, contents);
	}
	else
	{
	    const char* contents = va_arg((*pap), const char*);
	    void* client_data = va_arg((*pap), void *);

	    if(buttname == NULL)
		buttname = "button";

	    if (entry == gtMenuStandard)
	    {
		gtPushCB callback = va_arg((*pap), gtPushCB);

		entries[num_entries] = gtPushButton::create(
		    this, buttname, contents, callback, client_data);
	    }
	    else if (entry == gtMenuCheck)
	    {
		gtToggleCB callback = va_arg((*pap), gtToggleCB);

		entries[num_entries] = gtToggleButton::create(
		    this, buttname, contents, callback, client_data);
	    }
	    else if (entry == gtMenuCascade)
	    { 
		gtCascadeCB callback = va_arg((*pap), gtCascadeCB);

		cascades = (gtCascadeButton**)psetrealloc((char *)cascades,
			     (num_cascades + 1) * sizeof(gtCascadeButton*));

		cascades[num_cascades] = gtCascadeButton::create(
		    this, buttname, contents, callback, client_data);

		entries[num_entries] = cascades[num_cascades++];
	    }
	    XtSetArg(args[0], XmNmarginTop, 0);
	    XtSetArg(args[1], XmNmarginBottom, 0);
	    XtSetValues(entries[num_entries]->rep()->widget(), args, 2);
	}
	entries[num_entries++]->manage();
	ins_loc++;
    }
}


void gtOptionMenuXm::insert_entries(int loc ...)
{
    va_list ap;
    va_start(ap, loc);
    var_insert(loc, &ap);
    va_end(ap);
}


void gtOptionMenuXm::remove_entry(int loc)
{
    XtDestroyWidget(entries[loc]->rep()->widget());
    delete entries[loc];
    for(int n = loc; n < num_entries - 1; n++)
	entries[n] = entries[n + 1];
    num_entries--;
}


int gtOptionMenuXm::total_entries()
{
    return num_entries;
}


gtBase* gtOptionMenuXm::container()
{
    return pulldown_menu;
}


gtPulldownMenu* gtOptionMenuXm::sub_menu()
{
    return pulldown_menu;
}


gtPrimitive *gtOptionMenuXm::entry(int n)
{
    return entries[n];
}


gtCascadeButton* gtOptionMenuXm::cascade(int n)
{
    return cascades[n];
}


gtPrimitive* gtOptionMenuXm::button(const char *name)
{
    for (int n=0; n<num_entries; n++)
	if(strcmp(name, entries[n]->name()) == 0)
	    return entries[n];
    return NULL;
}


gtLabel* gtOptionMenuXm::label()
{
    return gt_casc;
}


void gtOptionMenuXm::menu_history(int index)
{
    Arg args[1];
    XtSetArg(args[0], XmNmenuHistory, entries[index]->rep()->widget());
    XtSetValues(rep()->widget(), args, 1);
}


void gtOptionMenuXm::menu_history(const char* name)
{
    Widget w = NULL;
    for(int i = 0; i < num_entries; i++)
    {
	if(strcmp(XtName(entries[i]->rep()->widget()), name) == 0)
	{
	    w = entries[i]->rep()->widget();
	    break;
	}
    }
    if(w)
    {
	Arg args[1];
	XtSetArg(args[0], XmNmenuHistory, w);
	XtSetValues(rep()->widget(), args, 1);
    }
}


char* gtOptionMenuXm::menu_history()
{
    Arg args[1];
    Widget w;
    XtSetArg(args[0], XmNmenuHistory, &w);
    XtGetValues(pulldown_menu->rep()->widget(), args, 1);

    return w ? XtName(w) : NULL;
}


void gtOptionMenuXm::move_to(gtEventPtr eve)
{
    XmMenuPosition(pulldown_menu->rep()->widget(), &(eve->xbutton));
}


void gtOptionMenuXm::map_callback(gtOptionCB callback, void *client_data)
{
    add_callback(XmNmapCallback, this, gtCallbackFunc(callback), client_data);
}


void gtOptionMenuXm::font (const char* name)
{
    XtVaSetValues (XmOptionButtonGadget (r->widget()),
	XtVaTypedArg, XmNfontList, XmRString, name, strlen (name), NULL);
}


void gtOptionMenuXm::_filler_()
{}


/*
   START-LOG-------------------------------------------

   $Log: gtOptionMenuXm.h.C  $
   Revision 1.7 2000/07/07 08:10:38EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.8  1994/04/13  14:17:17  builder
Port - use aset_stdarg first

Revision 1.2.1.7  1994/02/16  18:08:11  kws
Port

Revision 1.2.1.6  1994/01/13  02:46:20  kws
Use psetmalloc

Revision 1.2.1.5  1993/10/07  20:44:39  kws
Port

Revision 1.2.1.4  1993/01/26  04:47:58  glenn
Use gtCallbackMgrXm.
Add dtor.

Revision 1.2.1.3  1992/10/29  13:47:13  oak
Commented out deletion of the gt_casc label
in ~gtOptionMenuXm since it is already deleted
by the motif destruct call back.

   END-LOG---------------------------------------------
*/



