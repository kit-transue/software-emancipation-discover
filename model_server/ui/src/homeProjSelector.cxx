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
//
// homeProjSelector.C
//
//----------------------------------------------------------------------
// synopsis:
//
// Prompts user to select a home project on aset startup
//
//
// Current policy for home project selection: 
//
// 1. If -x (-home) is present, and specifies project which is writable and
// does not have children with pmods (at any level) (called eligible project
// in the sequel), then that project is accepted as home project.
//
// 2. If -x (-home) is present but either does not specify a project or
// specifies ineligible project, or specifies project whose ancestor has pmod,
// paraset exits and prints a message which explains why the requested project
// is not accepted as home project.
//
// 3. If -x (-home) is not present and there are no writable root projects in
// the .pdf file, paraset exits with message to that effect.
// 
// 4. If -x (-home) is not present, and there is exactly one writable root
// project in the .pdf file, then that project is taken as home project if it
// is eligible. Otherwise, if there is more than one writable project, or
// there is exactly one but that one is not eligible, a dialog box is offered.
// The box lists all writable projects in the .pdf file.
// 
// 5. When a selection in the dialog box is made, the logic similar to 4.  is
// recursively applied until selection if finally made. I.e., if the selected
// project is eligible, it is accepted. If it is not eligible, then it must
// have children with pmods. Therefore a dialog box offers writable children.
// If there are none, paraset exits with error message.
//
//
// This may not be perfect, but presently appears to prevent selection of
// ineligible projects as home projects. 
//
//                                           ljb, 05/18/1995
// 
//----------------------------------------------------------------------
//

// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <msg.h>
#include <messages.h>
#include <Question.h>

#include <genError.h>
#include <xref.h>
#include <symbolArr.h>
#include <RTLdialog.h>
#include <path.h>
#include <proj.h>
#include <projList.h>
#include <RTL_externs.h>

#include <gtBase.h>
#include <gtPushButton.h>
#include <gtRTL.h>

#include <top_widgets.h>

#include <machdep.h>
#include <driver_mode.h>
#include <feedback.h>

#include <mpb.h>

extern "C"
{
    void  driver_exit(int);
}

void logger_parent (gtDialogTemplate*);

// PRE-PROCESSOR DEFINITIONS
#define W_OK 2

// VARIABLE DEFINITIONS
static projNode *pr = NULL;
static int done = 0;

// FUNCTION DEFINITIONS
static int done_yet(void *cd) {
  return *(int *)cd;
}

int home_selector_close(void*)
{
  Initialize(home_selector_close);
  if (dis_question (T_NULL, B_OK, B_CANCEL, Q_QUITPARASET) > 0) {
    driver_exit (0);
    return 1;
  }
  return 0;
}

extern bool proj_is_writable(projNode *proj, genString& fn);

static void pick_project(int button, symbolArr* selection, void *gwritables, 
			 RTLdialog *rtl_dialog);

projNode* expand_to_include_child_projects(projNode *project, RTLdialog &choose_one,RTLPtr writables)
//
// Change the dialog contents set up by popup_home_proj_selector so that if the user
// tries to select a project one (or more) of whose children have pmod files in them
// then the children replace the parent in the dialog
//
{
    Initialize(expand_to_include_child_projects);

    projNode *pr   = NULL;
    done = 0;
    
    Obj* os = parentProject_get_childProjects(project);
    ObjPtr ob;
    ForEach(ob, *os) {
	projNode *nd = checked_cast(projNode, ob);
	
	char* name;
        name = nd->get_name();
	
  	if (strstr(name, "rule"))
	    continue;
	
	genString dummy_fn;

	if(proj_is_writable(nd, dummy_fn) && 
	   nd != projNode::get_control_project() )
	    rtl_add_obj(checked_cast(RTLNode, writables->get_root()), nd);
    }
    
    symbolArr *writables_list = &checked_cast(RTLNode,writables->get_root())
	->rtl_contents();
    if (writables_list->size() == 0)
	msg("Please define a writable home project in your pdf file.") << eom;
    else
    {
	projNode* the_only_writable_child = checked_cast(projNode,(appTreePtr)((*writables_list)[0]));
	if (writables_list->size() == 1 && Xref::test_subproj_pmod(the_only_writable_child) != -1) 
	    pr = the_only_writable_child;
#ifndef NEW_UI
	else
	    (choose_one.gtrtl())->regenerate_rtl(writables);
#endif
    }
    return pr;
}

static void pick_project(int button, symbolArr* selection,void *gwritables, 
		  RTLdialog *rtl_dialog)
// it analyzes the selection of the home project
//
// button=1: if ok is pressed
//       =0: if cancel is pressed
//
// selection: contains the selected project
//
// rtl_dialog: is the object of the dialog box
{
  underInitialize(pick_project);
  pr = NULL;
  symbolPtr sym;
  RTLPtr writables=*(RTLPtr *)gwritables;
  
  if(button == 1) {
    ForEachS(sym, *selection) {
      pr = checked_cast(projNode,(appTreePtr)sym);
    }

    if(pr == NULL) 
      msg("ERROR: You must choose a home project.") << eom;
    else if (Xref::test_subproj_pmod(pr)==-1) 
    {
        
	rtl_delete_obj(checked_cast(RTLNode,(*writables).get_root()),pr);
        push_busy_cursor();
	pr=expand_to_include_child_projects(pr,*rtl_dialog,writables);
	pop_cursor();
	
    }
    else {
      done = 1;
    }
  }

  if(button==-1) 
    home_selector_close(NULL);
}

static RTLdialog *choose_home = 0;

void delete_choose_home()
{
    Initialize(delete_choose_home);

    if (!choose_home)
        return;

    delete choose_home;
    choose_home = 0;
}

projNode *popup_home_proj_selector()
//
// create the dialog box for the selection of the home project
//
{
    Initialize(popup_home_proj_selector);
    pr   = NULL;
    done = 0;
    
#ifndef NEW_UI    
    RTLPtr writables = db_new(RTL,(0));
    
    int break_val = 8, val = 1,incr_val = 1, num_incs = 0;
    mpb_incr_values_init(break_val, val, incr_val);
    projNode *nd;
    for (int i=1; nd = projList::search_list->get_proj(i); i++)
    {
	genString dummy_fn;
	if(proj_is_writable(nd, dummy_fn) && 
	   nd != projNode::get_control_project() &&
	   nd->get_map()->get_view_flag()) 
	{
	    rtl_add_obj(checked_cast(RTLNode,writables->get_root()), nd);
	}
	
	mpb_step(break_val, incr_val, val, num_incs,i);
      }

    symbolArr *writables_list = &checked_cast(RTLNode,writables->get_root())
	->rtl_contents();
    if (writables_list->size() == 0)
	pr = NULL;
    else
    {
	projNode* the_only_root_project = checked_cast(projNode,(appTreePtr)((*writables_list)[0]));
	if (writables_list->size() == 1 && Xref::test_subproj_pmod(the_only_root_project) != -1) 
	    pr = the_only_root_project;
	else
	{
	    mpb_hide();
	    choose_home = new RTLdialog("choose_home", TXT("Home Project Selection"),
					pick_project,&writables, 3);
	    choose_home->init(TXT("Choose a home project:"), writables, 1);
	    choose_home->list_style(gtBrowse);
	    
	    // disable "Apply"
	    choose_home->button(1, 0, 0);
	    // disable "Help"
	    choose_home->button(3, 0, 0);
	    choose_home->override_WM_destroy(home_selector_close);
	    choose_home->popup();
	    gtBase::take_control(done_yet, (void *)&done);
	    mpb_popup();
	}
    }
    
    mpb_incr();

    if (!pr)
	msg("Please define a writable home project in your pdf file.") << eom;
#endif
    return pr;
}

