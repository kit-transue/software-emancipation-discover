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
// DC_Display_Selection.h.C
//------------------------------------------
// synopsis:
// DC_Display_Selection in libGT
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files
#include "machdep.h"
#include "DC_Display_Selection.h"

// external variables
static DC_Display_Selection_Ptr bds;

// external functions
extern "C" void apl_OODT_set_disp_selection(void *, void *, int, char *);
extern "C" int viewClass_mixin_get_member_filter(void *);
extern "C" char *viewClass_mixin_get_name_filter_text(void *);

extern "C" void popup_DC_Display_Selection(void *arg_viewp, void *pnode)
{
    if (!bds)
	bds = new DC_Display_Selection();
    bds->init(arg_viewp, pnode);
    bds->get_current_options();
#ifndef NEW_UI
    bds->shell->popup(0);
#endif
}

// pre-processor definitions

// variable definitions

// function definitions
void DC_Display_Selection::init(void *arg_viewp, void *pnode)
{
    popup_node = pnode;
    viewp = arg_viewp;
};

DC_Display_Selection::DC_Display_Selection()
{
#ifndef NEW_UI
    shell = gtDialogTemplate::create(NULL, "DC_Display_Selection",
            "Data Chart Display Selection");
    shell->add_default_buttons(
	NULL, &DC_Display_Selection::OK_CB, this,
	NULL, &DC_Display_Selection::Apply_CB, this,
	NULL, &DC_Display_Selection::Cancel_CB, this,
	NULL, NULL, NULL);
    shell->help_context_name("Designer.Dialogs.Help.DCDisplaySelection");

    filtslot = gtHorzBox::create(shell, "filtslot");
    tb3 = gtToggleButton::create(filtslot, "name_filter_toggle",
				 "Name Filter", NULL, NULL);
    tb3->manage();
    text1 = gtStringEditor::create(filtslot, "name_filter_text", NULL);
    text1->set_callback(&DC_Display_Selection::text_CB, this);
    text1->columns(51);
    text1->manage();	
    filtslot->attach_side_left();
    filtslot->attach_side_right();
    filtslot->attach_side_top();
    filtslot->attach_side_bottom();
    filtslot->manage();
#endif
};
    
void DC_Display_Selection::get_current_options()
{
#ifndef NEW_UI
   char* name_filt;
   name_filt = viewClass_mixin_get_name_filter_text(popup_node);
   if (name_filt)
     {
	text1->text(name_filt);
	tb3->set(1,1);
     }
   else
     {
	text1->text("");
	tb3->set(0,1);
     }
#endif
}

void DC_Display_Selection::do_it() {
#ifndef NEW_UI
  int use_name;
  use_name = tb3->set();
  apl_OODT_set_disp_selection(popup_node, viewp, show_methods | show_data |
        show_instance_members | show_public_members,
        use_name ? text1->text() : NULL);
#endif
}
    
void DC_Display_Selection::OK_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    DC_Display_Selection_Ptr BDS = DC_Display_Selection_Ptr(cd);

    BDS->do_it();
    BDS->shell->popdown();
#endif
};

void DC_Display_Selection::Apply_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    DC_Display_Selection_Ptr BDS = DC_Display_Selection_Ptr(cd);
    BDS->do_it();
#endif
};

void DC_Display_Selection::Cancel_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    DC_Display_Selection_Ptr BDS = DC_Display_Selection_Ptr(cd);
    BDS->shell->popdown();
#endif
};

void DC_Display_Selection::text_CB(gtStringEditor *se, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    DC_Display_Selection_Ptr BDS = DC_Display_Selection_Ptr(cd);
    char *txt;
    txt = se->text();
    if (txt && strcmp(txt, "") != 0)
	BDS->tb3->set(1,1);
    else
	BDS->tb3->set(0,1);
#endif
};
/*
   START-LOG-------------------------------------------

   $Log: DC_Display_Selection.h.C  $
   Revision 1.3 1996/08/12 16:47:05EDT pero 
   help_context_name() cannot be called before the corresponding          
   help_button is created (fixing all related problems to 12238)
// Revision 1.3  1993/06/18  21:22:17  wmm
// Hook up "help" buttons, change some terminology to be consistent with
// C++ and ParaSET usage.
//
// Revision 1.2  1993/02/06  22:45:36  wmm
// Fix bug that caused "member functions" of data chart structs to
// disappear permanently if a name filter were enabled (was looking
// only for data members in the data chart, duh).
//
// Revision 1.1  1992/12/17  21:32:22  wmm
// Initial revision
//

   END-LOG---------------------------------------------

*/
