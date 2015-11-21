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
// RTLdialog.h.C
//------------------------------------------
// synopsis:
// Dialog window containing a single run-time list and push-buttons
//------------------------------------------

// INCLUDE FILES

#include "machdep.h"
#include <genError.h>

#include <RTLdialog.h>

#include <RTL.h>
#include <gtDlgTemplate.h>
#include <gtForm.h>
#include <gtRTL.h>
#include <gtLabel.h>
#include <gtPushButton.h>

#define TXT(a) a


// FUNCTION DEFINITIONS

RTLdialog::RTLdialog(
    const char* name, const char* title,
    void (*cb)(int, symbolArr*, void*, RTLdialog*), void* cb_data, int m)
: callback(cb), callback_data(cb_data), modality(m), run_context(NULL),
								is_up (false)
{
    Initialize(RTLdialog::RTLdialog);
#ifndef NEW_UI
    shell = gtDialogTemplate::create(NULL, name, title);
    shell->add_default_buttons(ok_CB, this, apply_CB, this, cancel_CB, this);

    gt_label = gtLabel::create(shell, "label", TXT("List"));
    gt_label->alignment(gtBeginning);

    gt_rtl = gtRTL::create(shell, "rtl", NULL, gtBrowse, NULL, 0);
    gt_rtl->action_callback(action_CB, this);
    gt_rtl->num_rows(10);
    gt_rtl->attach(gtBottom);

    gtForm::vertStack(gt_label, gt_rtl->gt_list());

    gt_label->manage();
    gt_rtl->manage();
#endif
}


RTLdialog::~RTLdialog()
{
    Initialize(RTLdialog::~RTLdialog);
#ifndef NEW_UI
    shell->popdown();
    delete shell;
#endif
}


void RTLdialog::list_style(gtListStyle sty)
{
    Initialize(RTLdialog::list_style);
#ifndef NEW_UI
    gt_rtl->sel_type(sty);
#endif
}

void RTLdialog::init(const char* label, RTL* rtl, int sort_order)
{
    Initialize(RTLdialog::init);
#ifndef NEW_UI
    gt_label->text(label);
    gt_rtl->set_rtl(rtl, sort_order);
#endif
}

void RTLdialog::init (const char* label, symbolArr& arr, int sort_order)
{
    Initialize (RTLdialog::init);
#ifndef NEW_UI
    gt_label->text (label);
    gt_rtl->set_rtl (arr, sort_order);
#endif
}

void RTLdialog::button(int index, int visible, int enabled, const char* label)
{
    Initialize(RTLdialog::button);
#ifndef NEW_UI
    shell->show_button(index, visible);

    gtPushButton* b = shell->button(index);

    b->set_sensitive(enabled);

    if(label)
	b->title(label);
#endif
}


void RTLdialog::popup()
{
    Initialize(RTLdialog::popup);
#ifndef NEW_UI
    run_context = NULL;

    if (is_up)
	return;

    shell->popup(modality);
    is_up = true;
#endif
}


void RTLdialog::popdown()
{
    Initialize(RTLdialog::popdown);
#ifndef NEW_UI
    if (!is_up)
	return;

    shell->popdown();
    is_up = false;
#endif
}


void RTLdialog::push_button(int which_button)
{
    Initialize(RTLdialog::push_button);
#ifndef NEW_UI
    if(callback)
    {
	symbolArr selection;
	if(which_button >= 0)
	    gt_rtl->app_nodes(&selection);
	(*callback)(which_button, &selection, callback_data, this);
    }
#endif
}


void RTLdialog::ok_CB(gtPushButton*, gtEventPtr, void *cd, gtReason)
{
    Initialize(RTLdialog::ok_CB);
#ifndef NEW_UI
    ((RTLdialog*)cd)->push_button(1);
#endif
}

    
void RTLdialog::apply_CB(gtPushButton*, gtEventPtr, void *cd, gtReason)
{
    Initialize(RTLdialog::apply_CB);
#ifndef NEW_UI
    ((RTLdialog*)cd)->push_button(0);
#endif
}
    

void RTLdialog::cancel_CB(gtPushButton*, gtEventPtr, void *cd, gtReason)
{
    Initialize(RTLdialog::cancel_CB);
#ifndef NEW_UI
    ((RTLdialog*)cd)->push_button(-1);
#endif
}


void RTLdialog::action_CB(gtList*, gtEventPtr, void *cd, gtReason)
{
    Initialize(RTLdialog::action_CB);
#ifndef NEW_UI
    ((RTLdialog*)cd)->push_button(1);
#endif
}

void RTLdialog::override_WM_destroy(int (*callback)(void*))
{
  Initialize(RTdialog::override_WM_destroy);
   #ifndef NEW_UI 
  shell->override_WM_destroy(callback);
#endif
}
    
/*
   START-LOG-------------------------------------------

   $Log: RTLdialog.C  $
   Revision 1.5 2002/01/23 09:58:17EST ktrans 
   Merge from branch: mainly dormant code removal
// Revision 1.6  1993/07/26  13:16:25  swu
// added member function to override the WM close menu option
//
// Revision 1.5  1993/01/13  23:47:49  glenn
// Use show_button instead of manage/unmanage in button.
//
// Revision 1.4  1993/01/13  06:43:05  glenn
// Fix button() to do work that may not be immediately visible.
//
// Revision 1.3  1993/01/13  06:39:24  glenn
// Add button() for changing visibility, sensitivity, and label of buttons.
//
// Revision 1.2  1992/12/21  20:00:19  glenn
// Set up branch.
//
// Revision 1.1  1992/12/21  19:58:07  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

