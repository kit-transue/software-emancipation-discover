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
// Browser_Member_Sorting_Order.h.C
//------------------------------------------
// synopsis:
// Conversion of Browser_Member_Sorting_Order dialog to libGT
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files
#include <cLibraryFunctions.h>
#include <messages.h>

#include <Browser_Member_Sorting_Order.h>
#include <gtPushButton.h>

// external variables
static Browser_Member_Sorting_Order_Ptr bmso;
static const char* br_sort_stringlist[11] = {
    "Superclass Members",      
    "Local Members",	      
    "Methods",		      
    "Data Members",	      
    "Nested Types",	      
    "Class Members",	      
    "Instance Members",	      
    "Virtual Members",	      
    "Public Members",	      
    "Protected Members",	      
    "Private Members"	       
    };

// external functions
extern "C" void apl_OODT_setup_class_selection(void *, void *);
extern "C" void apl_OODT_reset_sorting_order();
extern "C" void apl_OODT_apply_sort(void *, int, int *);

extern "C" void popup_Browser_Member_Sorting_Order(void *arg_viewp, void *arg_popup_node)
{
    if (!bmso)
	bmso = new Browser_Member_Sorting_Order();
    bmso->init_fields(arg_viewp, arg_popup_node);
    bmso->update_depth();
#ifndef NEW_UI
    bmso->shell->popup(0);
#endif
};

// pre-processor definitions

// variable definitions

// function definitions

Browser_Member_Sorting_Order::Browser_Member_Sorting_Order(): map_count(0),
      sort_depth(0) {
    
#ifndef NEW_UI
    shell = gtDialogTemplate::create(NULL, "Browser_Member_Sorting_Order",
				     "Browser Member Sorting Order");
    shell->add_button("ok",     TXT("OK"), &Browser_Member_Sorting_Order::OK_CB, this);
    shell->add_button("apply",  TXT("Apply"), &Browser_Member_Sorting_Order::Apply_CB, this);
    shell->add_button("br_sort_reset", "Reset", &Browser_Member_Sorting_Order::Reset_CB,
		      this);
    shell->add_button("cancel", TXT("Cancel"),&Browser_Member_Sorting_Order::Cancel_CB,this);
    shell->add_help_button();
    shell->help_context_name("Designer.Dialogs.Help.BrowserMemberSortOrder");

    boxslot = gtVertBox::create(shell, "boxslot");
    rbox = gtRadioBox::create(
	boxslot, "rbox", NULL,
	"Inheritance", "br_sort_inheritance", &Browser_Member_Sorting_Order::Inheritance_CB,
	this,		      
	"Member Type", "br_sort_type", &Browser_Member_Sorting_Order::Type_CB, this,
	"Object Binding", "br_sort_binding", &Browser_Member_Sorting_Order::Binding_CB, this,
	"Member Access", "br_sort_access", &Browser_Member_Sorting_Order::Access_CB, this,
	NULL);
    rbox->manage();
    rbox2 = gtRadioBox::create(
	boxslot, "rbox2", NULL,
	"ASCII order", "br_sort_ascii", &Browser_Member_Sorting_Order::Ascii_CB, this,
	NULL);
    rbox2->manage();
    boxslot->attach_side_top();
    boxslot->attach_pos_left(10);
    boxslot->attach_pos_right(40);
    boxslot->attach_side_bottom();
    boxslot->manage();

    sel_list = gtList::create(
	shell, "sel_list", "Select item to change sorting priority", gtSingle, NULL, 0);
    sel_list->set_callback(&Browser_Member_Sorting_Order::List_CB, this);
    sel_list->num_rows(5);
    sel_list->attach_side_top();
    sel_list->attach_pos_left(50);
    sel_list->attach_pos_right(90);
    sel_list->manage();

//     sep = gtSeparator::create(shell, "sep", gtHORZ);
//     sep->attach_top(sel_list);
//     sep->attach_side_left();
//     sep->attach_side_right();
//     sep->manage();

    textslot = gtHorzBox::create(shell, "texstlot");
    label1 = gtLabel::create(textslot, "label1", "Depth:");
    text1 = gtStringEditor::create(textslot, "text1", NULL);
    text1->manage();
    textslot->attach_top(sel_list);
    textslot->attach_offset_top(40);
    textslot->attach_pos_left(50);
    textslot->attach_pos_right(90);
    textslot->manage();
#endif
};

void Browser_Member_Sorting_Order::init_fields(void *arg_viewp, void *arg_popup_node)
{
    i_popup_node = arg_popup_node;
    i_viewp = arg_viewp;
};

void Browser_Member_Sorting_Order::fill_selection_box()
{
#ifndef NEW_UI
    const char *blist[11];
    int i;
    for (i = 0; i < map_count; i++) {
	blist[i] = br_sort_stringlist[map_array[i]];
    };
    sel_list->delete_all_items();
    if (map_count) {
       sel_list->add_items(blist, map_count, 0);
    }
#endif
};

void Browser_Member_Sorting_Order::init_selection_box(int count, int min_idx, int type)
{
#ifndef NEW_UI
  int i;
  map_count = count;
  for (i = 0; i < count; i++)
    map_array[i] = min_idx + i;
  sort_type = type;
  min_map_index = min_idx;
  rbox2->button("br_sort_ascii")->set(0,0);
  fill_selection_box();
#endif
};

void Browser_Member_Sorting_Order::clear_fields()
{
#ifndef NEW_UI
  if (rbox->button("br_sort_inheritance")->set())
    init_selection_box(2,0, INHERITANCE_SORTER);
  else if (rbox->button("br_sort_type")->set())
    init_selection_box(3, 2, KIND_SORTER);
  else if (rbox->button("br_sort_binding")->set())
    init_selection_box(3, 5, CATEGORY_SORTER);
  else if (rbox->button("br_sort_access")->set())
    init_selection_box(3, 8, ACCESS_SORTER);
  else
    clear_pane_if_none();

#endif  
};

void Browser_Member_Sorting_Order::clear_pane_if_none()
{
#ifndef NEW_UI
  if (!(rbox->button("br_sort_inheritance")->set()) &&
      !(rbox->button("br_sort_type")->set()) &&
      !(rbox->button("br_sort_binding")->set()) &&
      !(rbox->button("br_sort_access")->set())) {
      sel_list->delete_all_items();
  }
#endif
};

void Browser_Member_Sorting_Order::update_depth()
{
#ifndef NEW_UI
  char buff[10];
  OSapi_sprintf(buff, "%d", sort_depth);
  text1->text(buff);
#endif
};

void Browser_Member_Sorting_Order::do_it()
{
  int i;
  for (i = 0; i < map_count; i++)
    map_array[i] -= min_map_index;
  apl_OODT_setup_class_selection(i_popup_node, i_viewp);
  if (sort_depth == 0)
    apl_OODT_reset_sorting_order();
  apl_OODT_apply_sort(i_viewp, sort_type, map_array);
  sort_depth++;
  update_depth();
  clear_fields();
};

void Browser_Member_Sorting_Order::process_selection()
{
  char* str;
  int i;
  int j;
  int temp;
  str = (sel_list->selected())[0];
  for (i = 0; i < map_count; i++) {
    if (strcmp(str, br_sort_stringlist[map_array[i]]) == 0) {
      for (j = i - 1; j >= 0; j--) {
        temp = map_array[j + 1];
        map_array[j + 1] = map_array[j];
        map_array[j] = temp;
      }
      sel_list->deselect_all();
      fill_selection_box();
      break;
    }
  }
};
    
void Browser_Member_Sorting_Order::OK_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Member_Sorting_Order_Ptr BMSO = Browser_Member_Sorting_Order_Ptr(cd);
    BMSO->do_it();
    BMSO->shell->popdown();
#endif
};

void Browser_Member_Sorting_Order::Apply_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Member_Sorting_Order_Ptr BMSO = Browser_Member_Sorting_Order_Ptr(cd);
    BMSO->do_it();
#endif
};

void Browser_Member_Sorting_Order::Cancel_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Member_Sorting_Order_Ptr BMSO = Browser_Member_Sorting_Order_Ptr(cd);
    BMSO->shell->popdown();
#endif
};

void Browser_Member_Sorting_Order::Reset_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Member_Sorting_Order_Ptr BMSO = Browser_Member_Sorting_Order_Ptr(cd);
    apl_OODT_setup_class_selection(BMSO->i_popup_node, BMSO->i_viewp);
    apl_OODT_reset_sorting_order();
    BMSO->sort_depth = 0;	
    BMSO->update_depth();
#endif
};

void Browser_Member_Sorting_Order::Inheritance_CB(gtToggleButton *tb, gtEventPtr, void *cd,
						  gtReason)
{
#ifndef NEW_UI
    Browser_Member_Sorting_Order_Ptr BMSO = Browser_Member_Sorting_Order_Ptr(cd);
    if (tb->set())
	BMSO->init_selection_box(2,0, INHERITANCE_SORTER);
    else
	BMSO->clear_pane_if_none();
#endif
};

void Browser_Member_Sorting_Order::Type_CB(gtToggleButton *tb, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Member_Sorting_Order_Ptr BMSO = Browser_Member_Sorting_Order_Ptr(cd);
    if (tb->set())
	BMSO->init_selection_box(3, 2, KIND_SORTER);
    else
	BMSO->clear_pane_if_none();
#endif
};

void Browser_Member_Sorting_Order::Binding_CB(gtToggleButton *tb, gtEventPtr,
					      void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Member_Sorting_Order_Ptr BMSO = Browser_Member_Sorting_Order_Ptr(cd);
    if (tb->set())
	BMSO->init_selection_box(3, 5, CATEGORY_SORTER);
    else
	BMSO->clear_pane_if_none();
#endif
};

void Browser_Member_Sorting_Order::Access_CB(gtToggleButton *tb, gtEventPtr, void *cd,
					     gtReason)
{
#ifndef NEW_UI
    Browser_Member_Sorting_Order_Ptr BMSO = Browser_Member_Sorting_Order_Ptr(cd);
    if (tb->set())
	BMSO->init_selection_box(3, 8, ACCESS_SORTER);
    else
	BMSO->clear_pane_if_none();
#endif
};

void Browser_Member_Sorting_Order::List_CB(gtList *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    Browser_Member_Sorting_Order_Ptr BMSO = Browser_Member_Sorting_Order_Ptr(cd);
    BMSO->process_selection();
#endif
};
void Browser_Member_Sorting_Order::clear_buttons()
{
#ifndef NEW_UI
  rbox->button("br_sort_inheritance")->set(0,0);
  rbox->button("br_sort_type")->set(0,0);
  rbox->button("br_sort_binding")->set(0,0);
  rbox->button("br_sort_access")->set(0,0);
  rbox2->button("br_sort_ascii")->set(0,0);
  sel_list->delete_all_items();
#endif
}

void Browser_Member_Sorting_Order::Ascii_CB(gtToggleButton *tb, gtEventPtr, void *cd,
					     gtReason)
{
#ifndef NEW_UI
    Browser_Member_Sorting_Order_Ptr BMSO = Browser_Member_Sorting_Order_Ptr(cd);
    if (tb->set()) {
	BMSO->clear_buttons();
	tb->set(1,0);
	BMSO->sort_type = ALPHA_SORTER;
    };
#endif
};

void Browser_Member_Sorting_Order::Declaration_CB(gtToggleButton *tb, gtEventPtr, void *cd,
					     gtReason)
{
#ifndef NEW_UI
    Browser_Member_Sorting_Order_Ptr BMSO = Browser_Member_Sorting_Order_Ptr(cd);
    if (tb->set()) {
	BMSO->clear_buttons();
	tb->set(1,0);
	BMSO->sort_type = DECL_ORDER_SORTER;
    };
#endif
};
/*
   START-LOG-------------------------------------------

   $Log: Browser_Member_Sorting_Order.h.C  $
   Revision 1.8 2000/07/07 08:16:49EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.4  1993/06/18  21:21:48  wmm
Hook up "help" buttons, change some terminology to be consistent with
C++ and ParaSET usage.

Revision 1.2.1.3  1993/05/02  19:47:42  wmm
Remove "Declaration Order" sort type, since DD does not support
it any more and there have been reports of crashes using it.

Revision 1.2.1.2  1992/10/09  19:54:35  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
