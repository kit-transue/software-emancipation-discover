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
// gtCascadeBXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtCascadeButton class in OSF Motif
//------------------------------------------

// INCLUDE FILES

#include <stdarg.h>

#include <gtBaseXm.h>
#include <gtPDMenuXm.h>
#include <gtCascadeBXm.h>

#include <Xm/CascadeB.h>


// FUNCTION DEFINITIONS

gtCascadeButton* gtCascadeButton::create(
    gtBase* parent, const char* name, const char* contents,
    gtCascadeCB callback, void* client_data)
{
    return new gtCascadeButtonXm(parent, name, contents, callback, client_data);
}


gtCascadeButtonXm::gtCascadeButtonXm(
    gtBase* p, const char* name, const char* contents,
    gtCascadeCB callback, void* client_data)
: parent(p), sub_m(NULL)
{
    Arg args[1];
    XmString st = XmStringCreateSimple((char*)gettext(contents));
    XtSetArg(args[0], XmNlabelString, st);
    r->widget(XmCreateCascadeButton(parent->container()->rep()->widget(),
				    (char*)name, args, 1));
    XmStringFree(st);

    if(callback != NULL)
	add_callback(XmNactivateCallback, this,
		     gtCallbackFunc(callback), client_data);

    destroy_init();
}


gtCascadeButtonXm::~gtCascadeButtonXm()
{}


void gtCascadeButtonXm::sub_menu(gtPrimitive* sub)
{
    Arg args[1];
    sub_m = (gtPulldownMenuXm*)sub;
    XtSetArg(args[0], XmNsubMenuId, sub->rep()->widget());
    XtSetValues(r->widget(), args, 1);
}


void gtCascadeButtonXm::label(const char* label_text)
{
    Arg args[1];
    XmString str = XmStringCreateSimple((char *)label_text);
    XtSetArg(args[0], XmNlabelString, str);
    XtSetValues(r->widget(), args, 1);
    XmStringFree(str);
}


void gtCascadeButtonXm::pulldown_menu(const char *name ...)
{
    va_list ap;
    va_start(ap, name);
    sub_m = new gtPulldownMenuXm(parent, name, &ap);
    va_end(ap);

    Arg args[1];
    XtSetArg(args[0], XmNsubMenuId, sub_m->rep()->widget());
    XtSetValues(r->widget(), args, 1);
}


void gtCascadeButtonXm::insert_entries(int loc ...)
{
    va_list ap;
    va_start(ap, loc);
    sub_m->var_insert(loc, &ap);
    va_end(ap);
}


void gtCascadeButtonXm::remove_entry(int loc)
{
    sub_m->remove_entry(loc);
}


gtPrimitive* gtCascadeButtonXm::entry(int n)
{
    return sub_m->entry(n);
}


gtCascadeButton* gtCascadeButtonXm::cascade(int n)
{
    return sub_m->cascade(n);
}


gtPrimitive* gtCascadeButtonXm::button(const char *c)
{
    return sub_m->button(c);
}


/*
   START-LOG-------------------------------------------

   $Log: gtCascadeBXm.h.C  $
   Revision 1.2 2000/07/07 08:10:30EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.5  1994/04/13  14:17:17  builder
Port - use aset_stdarg first

Revision 1.2.1.4  1993/10/07  20:44:39  kws
Port

Revision 1.2.1.3  1993/01/26  04:32:20  glenn
Remove construct member.
Use gtCallbackMgrXm.

Revision 1.2.1.2  1992/10/09  19:03:26  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
