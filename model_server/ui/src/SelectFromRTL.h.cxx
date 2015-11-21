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
// SelectFromRTL.h.C
//------------------------------------------
// synopsis:
// SelectFromRTL dialog in libGT
//
//------------------------------------------

// INCLUDE FILES

#include "machdep.h"
#include <genError.h>

#include <SelectFromRTL.h>
#include <gtRTL.h>
#include <gtDlgTemplate.h>
#include <gtRTL.h>
#include <gtStringEd.h>
#include <gtLabel.h>
#include <gtPushButton.h>
#include "RTL.h"

#define TXT(a) a

// FUNCTION DEFINITIONS

SelectFromRTL::SelectFromRTL(
    int modal, const char* title,
    void (*callback)(void*, int, int, const char *), void* callback_data)
: select_callback(callback), select_callback_data(callback_data),
  confirm(0), modality(modal), text_editor_enabled(1)
{
#ifndef NEW_UI
    shell = gtDialogTemplate::create(
	NULL, "select_from_rtl", title ? title : TXT("Select"));

    list_label = gtLabel::create(shell, "list_label", TXT("List"));
    list_label->attach(gtLeft);
    list_label->attach(gtRight);
    list_label->attach(gtTop);
    list_label->manage();

    select_text = gtStringEditor::create(shell, "select_text", "");
    select_text->attach(gtLeft);
    select_text->attach(gtRight);
    select_text->attach(gtBottom);
    select_text->manage();

    select_label = gtLabel::create(shell, "select_label", TXT("Selection"));
    select_label->attach(gtLeft);
    select_label->attach(gtRight);
    select_label->attach(gtBottom, select_text);
    select_label->manage();

    rtlist = gtRTL::create(shell, "rtl", NULL, gtBrowse, NULL, 0);
    rtlist->select_callback(List_CB, this);
    rtlist->action_callback(Action_CB, this);
    rtlist->num_rows(10);
    rtlist->gt_list()->width(800);
    rtlist->attach(gtLeft);
    rtlist->attach(gtRight);
    rtlist->attach(gtTop, list_label);
    rtlist->attach(gtBottom, select_label);
    rtlist->manage();
#endif
}

SelectFromRTL::~SelectFromRTL()
{
#ifndef NEW_UI
    shell->popdown();
    delete shell;
#endif
}

void SelectFromRTL::buttons(const char* ok_label,
			    const char* apply_label,
			    const char* cancel_label)
{
    Initialize(SelectFromRTL::buttons);
#ifndef NEW_UI
    if(ok_label)
	shell->add_button("ok", ok_label, OK_CB, this);
    if(apply_label)
	shell->add_button("apply", apply_label, Apply_CB, this);
    if(cancel_label)
	shell->add_button("cancel", cancel_label, Cancel_CB, this);

    shell->add_help_button();
#endif
    Return
}

void SelectFromRTL::help_context_name(const char* context)
{
#ifndef NEW_UI
    shell->help_context_name(context);
#endif
}

void SelectFromRTL::mode(gtListStyle sty, int enable_text_editor)
{
    Initialize(SelectFromRTL::mode);
#ifndef NEW_UI
    rtlist->sel_type(sty);

    if(sty == gtMultiple || sty == gtExtended)
	enable_text_editor = 0;

    if(text_editor_enabled = enable_text_editor)
    {
	select_text->manage();
	select_label->manage();

	rtlist->unmanage();
	rtlist->attach(gtBottom, select_label);
	rtlist->manage();
    }
    else
    {
	rtlist->unmanage();
	rtlist->attach(gtBottom);
	rtlist->manage();

	select_text->unmanage();
	select_label->unmanage();
    }
#endif
    Return
}

void SelectFromRTL::init(
    const char *label, void *rtl, int sort_order)
{
    Initialize(SelectFromRTL::init);
#ifndef NEW_UI
    list_label->text(label);
    rtlist->set_rtl(checked_cast(RTL,rtl), sort_order);
#endif
    Return
}

void SelectFromRTL::popup()
{
    Initialize(SelectFromRTL::popup);
#ifndef NEW_UI
    shell->popup(0); // should use modality.
#endif
    Return
}

void SelectFromRTL::popdown()
{
    Initialize(SelectFromRTL::popdown);
#ifndef NEW_UI
    shell->popdown();
#endif
    Return
}

void* SelectFromRTL::run(objArr* app_nodes)
{
    Initialize(*SelectFromRTL::run);
#ifndef NEW_UI
    shell->popup(0); // should use modality
    confirm = 0;
    gtBase::take_control(&SelectFromRTL::confirm_func, this);
    shell->popdown();

    ReturnValue((confirm > 0) ? rtlist->app_nodes(app_nodes) : NULL);
#else
    return NULL;
#endif
}
	
void SelectFromRTL::selection(int on)
{
    Initialize(SelectFromRTL::selection);
#ifndef NEW_UI
    if (on) {
	select_label->manage();
	select_text->manage();
    } else {
	select_label->unmanage();
	select_text->unmanage();
    }
#endif
    Return
}

void SelectFromRTL::do_it(int from_apply)
{
    Initialize(SelectFromRTL::do_it);
#ifndef NEW_UI
    confirm = 1;
    if(select_callback)
    {
	if(text_editor_enabled)
	{
	    char* value = select_text->text();
	    (*select_callback)(select_callback_data, from_apply,
			       rtlist->item_pos(value), value);
	    free(value);
	}
	else
	{
	    if(select_callback_data)
		rtlist->app_nodes((objArr*)select_callback_data);

	    (*select_callback)(select_callback_data, from_apply, -1, NULL);
	}
    }
#endif
    Return
}

void SelectFromRTL::List_CB(gtList *l, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    int *pos, num;
    l->get_selected_pos(&pos, &num);
    if (num == 1)
    {
	char *txt = (l->selected())[0];
	((SelectFromRTL_Ptr)cd)->select_text->text(txt);
    }
#endif
}

void SelectFromRTL::Action_CB(gtList *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    SelectFromRTL_Ptr(cd)->do_it(1);
#endif
}

void SelectFromRTL::OK_CB(gtPushButton*, gtEventPtr, void *cd, gtReason)
{
    Initialize(SelectFromRTL::OK_CB);
#ifndef NEW_UI
    SelectFromRTL_Ptr(cd)->do_it(0);
#endif
    Return
}
    
void SelectFromRTL::Apply_CB(gtPushButton*, gtEventPtr, void *cd, gtReason)
{
    Initialize(SelectFromRTL::Apply_CB);
#ifndef NEW_UI
    SelectFromRTL_Ptr(cd)->do_it(1);
#endif
    Return
}
    
void SelectFromRTL::Cancel_CB(gtPushButton*, gtEventPtr, void *cd, gtReason)
{
    Initialize(SelectFromRTL::Cancel_CB);
#ifndef NEW_UI
    SelectFromRTL_Ptr SFP = SelectFromRTL_Ptr(cd);
    SFP->confirm = -1;
    SFP->shell->popdown();
#endif
    Return
}

int SelectFromRTL::confirm_func(void* cd)
{
    Initialize(SelectFromRTL::confirm_func);
#ifndef NEW_UI
    ReturnValue(SelectFromRTL_Ptr(cd))->confirm;
#else
    return 0;
#endif
}

/*
   START-LOG-------------------------------------------

   $Log: SelectFromRTL.h.C  $
   Revision 1.2 1996/06/28 18:21:50EDT mstarets 
   NT branch merge
Revision 1.2.1.6  1993/06/25  13:30:00  andrea
fixed bug 3690 (errorBrowser gets wider than screen)

Revision 1.2.1.5  1993/04/28  12:40:15  jon
Added member function SelectFromRTL::help_context_name()

Revision 1.2.1.4  1993/01/26  19:36:50  jon
Added action callback to fix bug #1534

Revision 1.2.1.3  1992/11/23  21:59:34  wmm
typesafe casts.
.`

Revision 1.2.1.2  1992/10/09  20:13:43  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
