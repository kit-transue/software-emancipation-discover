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
// Browser_Display_Selection.h.C
//------------------------------------------
// synopsis:
// Browser_Display_Selection in libGT
//
// description:
// The class Browser_Display_Selection represents the dialog box
// that comes up when Relations=>Show Members or Inheritance=>Show members
// is selected fro and ERD or an inheritance view. The file was initially
// set up to contain a single staic object 'bds' which would be created at the
// first selection and remain untill the viewershell is up.
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files
#include "machdep.h"
#include <Browser_Display_Selection.h>
#include <gtPushButton.h>
#include <objRelation.h>
#include <viewERDHeader.h>

// external variables
static Browser_Display_Selection_Ptr bds;
static Browser_Display_Selection_Ptr bds_erd;

// external functions
extern "C" void apl_OODT_set_disp_selection(void *, void *, int, char *);
extern "C" int viewClass_mixin_get_member_filter(void *);
extern "C" char *viewClass_mixin_get_name_filter_text(void *);


//bugfix 7252. 7/7/94
//In order to distinguish between the dialog box for ERDs and Inheritance view,
//a second static ptr bds_erd is  introduced and an additional parameter is
// passed to the constructor of  Browser_Display_Selection to allow it 
//determine what type of dialog box to create. Therefore, depending on the
// type of view, the correct dlg box is created and subsequently modified.

extern "C" void popup_Browser_Display_Selection(void *arg_viewp, void *pnode)
//Checks who the caller is -ie., what type of view and initializes 
//appropriate ptr. bds - for Inheritance view
// bds_erd - for ERD views.
{
    Initialize(popup_Browser_Display_Selection);
    if(is_viewERDHeader(checked_cast(viewGraHeader,arg_viewp))){
	if(!bds_erd)
	    bds_erd = new Browser_Display_Selection((viewGraHeader*)arg_viewp);
	 bds_erd->init(arg_viewp, pnode);
         bds_erd->get_current_options();
#ifndef NEW_UI
         bds_erd->shell->popup(0);
#endif
    }else{
	if(!bds)
	    bds = new Browser_Display_Selection((viewGraHeader*)arg_viewp); 
	bds->init(arg_viewp, pnode);
        bds->get_current_options();
#ifndef NEW_UI
        bds->shell->popup(0);
#endif
    }
 
}

// pre-processor definitions

// variable definitions

// function definitions
void Browser_Display_Selection::init(void *arg_viewp, void *pnode)
{
    popup_node = pnode;
    viewp = arg_viewp;
	notifier = psetNotifyView::construct((view*)arg_viewp, view_deleted_CB, this);
};

Browser_Display_Selection::Browser_Display_Selection(viewGraHeader* arg_viewp)
{
    Initialize(Browser_Display_Selection::Browser_Display_Selection);
#ifndef NEW_UI
    viewp = arg_viewp;
    shell = gtDialogTemplate::create(NULL, "Browser_Display_Selection", "Show Members");
    shell->add_default_buttons(
	NULL, &Browser_Display_Selection::OK_CB, this,
	NULL, &Browser_Display_Selection::Apply_CB, this,
	NULL, &Browser_Display_Selection::Cancel_CB, this,
	NULL, NULL, NULL);
    shell->help_context_name("Designer.Dialogs.Help.BrowserDisplaySelection");

    toplabel = gtLabel::create(shell, "instructions",
			       "Choose at least one item from each column");
    toplabel->attach_side_top();
    toplabel->attach_offset_top(8);
    toplabel->attach_pos_left(25);
    toplabel->attach_pos_right(75);

    togslot = gtHorzBox::create(shell, "togslot");
    tog1 = gtToggleBox::create(
	togslot, "tog1", NULL,
	"Methods", "ds_methods", &Browser_Display_Selection::sanity_CB, this,
	"Data", "ds_data", &Browser_Display_Selection::sanity_CB, this,
	"Nested Types", "ds_types", &Browser_Display_Selection::sanity_CB, this,
	NULL);
    tog1->manage();
    tog2 = gtToggleBox::create(
	togslot, "tog2", NULL,
	"Class Members", "ds_class", &Browser_Display_Selection::sanity_CB, this,
	"Instance Members", "ds_instance", &Browser_Display_Selection::sanity_CB, this,
	"Virtual Members", "ds_virtual", &Browser_Display_Selection::sanity_CB, this,
	NULL);
    tog2->manage();
    tog3 = gtToggleBox::create(
	togslot, "tog3", NULL,
	"Public Members", "ds_public", &Browser_Display_Selection::sanity_CB, this,
	"Protected Members", "ds_protected", &Browser_Display_Selection::sanity_CB, this,
	"Package Access Members", "ds_package_prot", &Browser_Display_Selection::sanity_CB, this,
	"Private Members", "ds_private", &Browser_Display_Selection::sanity_CB, this,
	NULL);
    tog3->manage();
    togslot->attach_side_left();
    togslot->attach_side_right();
    togslot->attach_top(toplabel);
    togslot->manage();

    togslot2 = gtHorzBox::create(shell, "togslot2");
    
    tb1 = NULL;
    if(!is_viewERDHeader(checked_cast(viewGraHeader,viewp)))
    {  //don`t create this toggle button if ERD view
    
       tb1 = gtToggleButton::create(togslot2, "ds_inherited",
				 "Inherited Members", NULL, NULL);
       tb1->manage();
    }

    if(!is_viewERDHeader(checked_cast(viewGraHeader,viewp)))
    {  //don`t create this toggle button if ERD view
       tb2 = gtToggleButton::create(togslot2, "ds_arguments",
				 "Method Arguments", NULL, NULL);
       tb2->manage();
    }       
    
    togslot2->attach_side_left();
    togslot2->attach_side_right();
    togslot2->attach_top(togslot);
    togslot2->manage();

    filtslot = gtHorzBox::create(shell, "filtslot");
    tb3 = gtToggleButton::create(filtslot, "name_filter_toggle",
				 "Name Filter", NULL, NULL);
    tb3->manage();
    text1 = gtStringEditor::create(filtslot, "name_filter_text", NULL);
    text1->set_callback(&Browser_Display_Selection::text_CB, this);
    text1->columns(51);
    text1->manage();	
    filtslot->attach_side_left();
    filtslot->attach_side_right();
    filtslot->attach_top(togslot2);
    filtslot->attach_side_bottom();
    filtslot->manage();
#endif
};
    
void Browser_Display_Selection::sanity_CB(gtToggleButton *tb, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Display_Selection_Ptr BDS = Browser_Display_Selection_Ptr(cd);
    BDS->sanity_check();
#endif
};

void Browser_Display_Selection::sanity_check()
{
    int first_group, second_group, third_group;
#ifndef NEW_UI
  first_group = tog1->button("ds_methods")->set() ||
      tog1->button("ds_data")->set() ||
      tog1->button("ds_types")->set();
  second_group = tog2->button("ds_class")->set() ||
      tog2->button("ds_instance")->set() ||
      tog2->button("ds_virtual")->set();
  third_group = tog3->button("ds_public")->set() ||
      tog3->button("ds_protected")->set() ||
      tog3->button("ds_package_prot")->set() ||
      tog3->button("ds_private")->set();
  if (first_group && second_group && third_group) {
      shell->button("ok")->set_sensitive(1);
      shell->button("apply")->set_sensitive(1);
  }
  else
  {
      shell->button("ok")->set_sensitive(0);
      shell->button("apply")->set_sensitive(0);
  }
#endif
};

void Browser_Display_Selection::get_current_options()
{
  Initialize(Browser_Display_Selection::get_current_options);
#ifndef NEW_UI
  int filter;
  char* name_filt;
  filter = viewClass_mixin_get_member_filter(popup_node);
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

  if (filter & show_methods)
    tog1->button("ds_methods")->set(1, 1);
  if (filter & show_data)
    tog1->button("ds_data")->set(1, 1);
  if (filter & show_nested_types)
    tog1->button("ds_types")->set(1, 1);
  if (filter & show_class_members)
    tog2->button("ds_class")->set(1, 1);
  if (filter & show_instance_members)
    tog2->button("ds_instance")->set(1, 1);
  if (filter & show_virtual_members)
    tog2->button("ds_virtual")->set(1, 1);
  if (filter & show_public_members)
    tog3->button("ds_public")->set(1, 1);
  if (filter & show_protected_members)
    tog3->button("ds_protected")->set(1, 1);
  if (filter & show_package_prot_members)
    tog3->button("ds_package_prot")->set(1, 1);
  if (filter & show_private_members)
    tog3->button("ds_private")->set(1, 1);
  if(!is_viewERDHeader(checked_cast(viewGraHeader,viewp)))
  {				//tb1 toggle doesn`t exist if ERD view
    if (filter & show_inherited)
      tb1->set(1, 1);
  }
  if(!is_viewERDHeader(checked_cast(viewGraHeader,viewp)))
  {				//tb2 toggle doesn`t exist if ERD view
    if (filter & show_arguments)
      tb2->set(1, 1);
  }
  sanity_check();
#endif
}

void Browser_Display_Selection::do_it() {
    
    Initialize(Browser_Display_Selection::do_it);
#ifndef NEW_UI
  int filter;
  int use_name;

  use_name = tb3->set();
  filter = 0;
  if (tog1->button("ds_methods")->set())
    filter |= show_methods;
  if (tog1->button("ds_data")->set())
    filter |= show_data;
  if (tog1->button("ds_types")->set())
    filter |= show_nested_types;
  if (tog2->button("ds_class")->set())
    filter |= show_class_members;
  if (tog2->button("ds_instance")->set())
    filter |= show_instance_members;
  if (tog2->button("ds_virtual")->set())
    filter |= show_virtual_members;
  if (tog3->button("ds_public")->set())
    filter |= show_public_members;
  if (tog3->button("ds_protected")->set())
    filter |= show_protected_members;
  if (tog3->button("ds_package_prot")->set())
    filter |= show_package_prot_members;
  if (tog3->button("ds_private")->set())
    filter |= show_private_members;
  if(!is_viewERDHeader(checked_cast(viewGraHeader,viewp)))
  { //tb1 toggle doesn\'t exist if ERD view
    if (tb1->set())
      filter |= show_inherited;
  }
  if(!is_viewERDHeader(checked_cast(viewGraHeader,viewp)))
  { //tb2 toggle doesn\'t exist if ERD view
     if (tb2->set())
       filter |= show_arguments;
  }
  apl_OODT_set_disp_selection(popup_node, viewp, filter,
	use_name ? text1->text() : NULL);
#endif
}
    
void Browser_Display_Selection::OK_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Display_Selection_Ptr BDS = Browser_Display_Selection_Ptr(cd);

    BDS->do_it();

	if (BDS->notifier) {
		BDS->notifier->destruct(BDS->notifier);
		BDS->notifier = NULL;
	}

    BDS->shell->popdown();
#endif
};

void Browser_Display_Selection::Apply_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Display_Selection_Ptr BDS = Browser_Display_Selection_Ptr(cd);
    BDS->do_it();
#endif
};

void Browser_Display_Selection::Cancel_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Display_Selection_Ptr BDS = Browser_Display_Selection_Ptr(cd);

	if (BDS->notifier) {
		BDS->notifier->destruct(BDS->notifier);
		BDS->notifier = NULL;
	}

    BDS->shell->popdown();
#endif
};

void Browser_Display_Selection::text_CB(gtStringEditor *se, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Display_Selection_Ptr BDS = Browser_Display_Selection_Ptr(cd);
    char *txt;
    txt = se->text();
    if (txt && strcmp(txt, "") != 0)
	BDS->tb3->set(1,1);
    else
	BDS->tb3->set(0,1);
#endif
};


// This function will be called if the view is deleted before the popup is closed.
void Browser_Display_Selection::view_deleted_CB(void* context, psetNotifyView* obj) 
{
#ifndef NEW_UI
    Browser_Display_Selection_Ptr BDS = (Browser_Display_Selection_Ptr) context;
	if (BDS) BDS->shell->popdown();
#endif
}

