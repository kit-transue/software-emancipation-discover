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
// Filter_Relations.h.C
//------------------------------------------
// synopsis:
// interface for the Add Relation dialog box with libGT calls
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files
#include <cLibraryFunctions.h>
#include <Filter_Relations.h>
#include <gtPushButton.h>

#ifndef _OODT_apl_entries_h
#include <OODT_apl_entries.h>
#endif

// variable definitions
static Filter_Relations_Ptr frp;

// function definitions

extern "C" char *viewClass_mixin_get_relation_filter_text(void *class_mixin);  
extern "C" void popup_filter_relations(void *arg_class_mix, void *arg_vh)
{
    if (!frp)
	frp = new Filter_Relations();
    frp->init_fields(arg_class_mix, arg_vh);
#ifndef NEW_UI
    frp->shell->popup(1);
#endif
};

Filter_Relations::Filter_Relations()
{
#ifndef NEW_UI
    shell = gtDialogTemplate::create(NULL, "Filter_Relations", "Filter Relations");
    shell->add_default_buttons(
	NULL, &Filter_Relations::OK_CB, this,
	NULL, &Filter_Relations::Apply_CB, this,
	NULL, &Filter_Relations::Cancel_CB, this,
	NULL, NULL, NULL);
    shell->help_context_name("Designer.Dialogs.Help.FilterRelations");

    textslot = gtHorzBox::create(shell, "textslot");
    filter_toggle = gtToggleButton::create(textslot, "filter_toggle", /*T*/"Filter",
					   NULL, NULL);
    filter_toggle->manage();
    filter_text = gtStringEditor::create(textslot, "filter_text", NULL);
    filter_text->set_callback(&Filter_Relations::Text_CB, this);
    filter_text->manage();
    textslot->attach_side_top();
    textslot->attach_offset_top(10);
    textslot->attach_side_left();
    textslot->attach_offset_left(10);
    textslot->manage();

    toggleButton1 = gtToggleButton::create(shell, "toggleButton1",
					   /*T*/"Show Inherited Relations", NULL, NULL);
    toggleButton1->attach_top(textslot);
    toggleButton1->attach_offset_top(10);
    toggleButton1->attach_side_left();
    toggleButton1->attach_offset_left(10);
    toggleButton1->attach_side_bottom();
    toggleButton1->attach_offset_bottom(10);
    toggleButton1->manage();
#endif
};

void Filter_Relations::init_fields(void *arg_class_mix, void *arg_vh)
{
#ifndef NEW_UI
    class_mixin = arg_class_mix;
    viewhead = arg_vh;
    char *filter_value = viewClass_mixin_get_relation_filter_text(class_mixin);
    if (filter_value)
	filter_text->text(filter_value);
#endif
};

void Filter_Relations::do_it()
{
#ifndef NEW_UI
    char *txt;
    txt = filter_toggle->set() ? filter_text->text() : NULL;
    apl_OODT_set_relation_filter((viewClass_mixin*) class_mixin, (viewGraHeader*) viewhead,
				 txt, toggleButton1->set());
#endif
};

void Filter_Relations::OK_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{ 
#ifndef NEW_UI
    Filter_Relations_Ptr FRP = Filter_Relations_Ptr(cd);
    FRP->do_it();
    FRP->shell->popdown();
#endif
};

void Filter_Relations::Apply_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Filter_Relations_Ptr FRP = Filter_Relations_Ptr(cd);
    FRP->do_it();
#endif
};
    
void Filter_Relations::Cancel_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
   Filter_Relations_Ptr(cd)->shell->popdown();
#endif
};

void Filter_Relations::Text_CB(gtStringEditor *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Filter_Relations_Ptr FRP = Filter_Relations_Ptr(cd);
    char *txt;
    txt = FRP->filter_text->text();
    if (txt && strcmp(txt, "") != 0)
	FRP->filter_toggle->set(1,1);
    else
	FRP->filter_toggle->set(0,1);
#endif
};


/*
   START-LOG-------------------------------------------

   $Log: Filter_Relations.h.C  $
   Revision 1.5 1996/08/12 16:49:18EDT pero 
   help_context_name() cannot be called before the corresponding          
   help_button is created (fixing all related problems to 12238)
Revision 1.2.1.3  1993/06/18  21:22:47  wmm
Hook up "help" buttons, change some terminology to be consistent with
C++ and ParaSET usage.

Revision 1.2.1.2  1992/10/09  19:54:47  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
