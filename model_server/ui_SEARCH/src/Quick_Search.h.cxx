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
// Quick_Search.h.C
//------------------------------------------
// Quick_Search dialog box
//------------------------------------------

// include files
#include <cLibraryFunctions.h>
#include <objOper.h>
#include <Quick_Search.h>

#include <malloc.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <genError.h>
#include <systemMessages.h>
#include <representation.h>
#include <top_widgets.h>
#include <smt.h>
#include <steView.h>
#include <steRawView.h>
#include <gtDlgTemplate.h>
#include <gtLabel.h>
#include <gtSepar.h>
#include <gtPushButton.h>
#include <gtTogB.h>
#include <gtToggleBox.h>
#include <gtHorzBox.h>
#include <gtStringEd.h>
#include <gtForm.h>
#include <gtScale.h>
#include <objCollection.h>

// external functions
extern "C" void *viewerShell_get_current_view(void *);
extern "C" void viewerShell_set_current_marker_categ(void *, char *);

extern "C" void popup_quick_search()
{ new Quick_Search(); }

// variable definitions

objArr Quick_Search::instance_list;
viewPtr Quick_Search::curr_view = NULL;

#define NUM_CONSTRAINT_BUTTONS 12

const struct
{
    const char *name;
    Constraints constraint_flag;
    boolean smt_only;
} button_info[NUM_CONSTRAINT_BUTTONS] =
       {
        {"toggle_reset",  (Constraints)0,0},
        {"vdecl",         SRCH_VDECL,    1},
	{"fdecl",         SRCH_FDECL,    1},
	{"pdecl",         SRCH_PDECL,    1},
	{"cdecl",         SRCH_CDECL,    1},
	{"edecl",         SRCH_EDECL,    1},
	{"keyword",       SRCH_KEYWORD,  1},
	{"field",         SRCH_FIELD,    1},
	{"macro",         SRCH_MACRO ,   1},
        {"const",         SRCH_CONST,    1},
        {"string",        SRCH_STRING,   1},
        {"comment",       SRCH_COMMENT,  1},
};

// function definitions
Quick_Search::Quick_Search() : search(NULL)
{
    inst.prev_string      = NULL;
    inst.prev_view        = NULL;
    inst.prev_constraints = 0;
    inst.match_count      = 0;
    inst.current_count    = 0;

    shell = gtDialogTemplate::create(NULL, "quick_search",
				     TXT("Entity Search"));

    shell->add_button("next", "First", &Quick_Search::Next_CB, this);
    shell->add_button("prev", "Last", &Quick_Search::Previous_CB, this);
    shell->add_button("done", "Done", &Quick_Search::Done_CB, this);
    shell->add_help_button();
    //Guy: I am adding a help link
    shell->help_context_name("aset.quick_search_popup.quick_search.help");

    search_lab = gtLabel::create(shell, "search_label", TXT("Search Pattern"));
    search_lab->alignment(gtBeginning);

    search_text = gtStringEditor::create(shell, "search_text", "");
    search_text->changed_callback(&Quick_Search::Changed_CB, this);

    tbox1 = gtToggleBox::create(
        shell, "tbox1", NULL,
	TXT("Case Insensitive"), "insensitive", &Quick_Search::Case_CB, this,
//	TXT("Match Words"), "word_match", &Quick_Search::Match_CB, this,
        NULL);
    tbox1->num_columns(2);

    tbox2 = gtToggleBox::create(
	shell, "tbox2", NULL,
	TXT("Find Any"),         button_info[0].name, Constraint_CB, this,
	TXT("Variable"),         button_info[1].name, Constraint_CB, this,
	TXT("Function"),         button_info[2].name, Constraint_CB, this,
	TXT("Parameter"),        button_info[3].name, Constraint_CB, this,
	TXT("Class/Struct"),     button_info[4].name, Constraint_CB, this,
	TXT("Enumeration"),      button_info[5].name, Constraint_CB, this,
	TXT("Keyword"),          button_info[6].name, Constraint_CB, this,
        TXT("Field"),            button_info[7].name, Constraint_CB, this,
	TXT("Macro"),            button_info[8].name, Constraint_CB, this,
	TXT("Const"),            button_info[9].name, Constraint_CB, this,
	TXT("String"),           button_info[10].name, Constraint_CB, this,
	TXT("Comment"),          button_info[11].name, Constraint_CB, this,
	NULL);

    last_attributes_setting = 1;
    last_enable_dialog = 1;
    tbox2->num_columns(4);
    tbox2->button(button_info[0].name)->set(1, 0);

    gtForm::vertStack(search_lab, search_text, tbox1, tbox2);
    search_lab->manage();
    search_text->manage();
    tbox1->manage();
    tbox2->manage();

    matches_label = gtLabel::create(shell, "matches_label", TXT("Occurrences Found: "));
    matches_label->alignment(gtBeginning);
    matches_label->attach(gtLeft);
    matches_label->attach(gtTop, tbox2, 5);
    matches_label->manage();

    matches = gtLabel::create(shell, "matches", "0");
    matches->alignment(gtBeginning);
    matches->attach(gtLeft, matches_label, 10);
    matches->attach(gtTop, tbox2, 5);
    matches->manage();

    instance_list.insert_last(this);

    set_widgets();

    shell->popup(0);
}

Quick_Search::~Quick_Search()
{
    if (search)
	delete search;
    shell->popdown();
    instance_list.remove((Obj *)this);
    delete shell;
}

void Quick_Search::update_search_dialogs(void *new_view)  // static
{
    curr_view = (viewPtr)new_view;
    Obj* el;
    ForEach(el, Quick_Search::instance_list)
    {
	Quick_Search * instance = (Quick_Search *)el;
	instance->view_changed();
    }
}

int Quick_Search::theConstraints()
{
    int constraints = 0;
    for (int i=1; i<NUM_CONSTRAINT_BUTTONS; i++)
	if (tbox2->button(button_info[i].name)->set())
	    constraints |= button_info[i].constraint_flag;

    if (tbox1->button("insensitive")->set())
	constraints |= SRCH_INEXACT;
//    if (tbox1->button("word_match")->set())
//	constraints |= SRCH_WORD;

    return constraints;
}

boolean Quick_Search::new_search_needed(boolean reset)
{
    int new_search = 0;

    if (!curr_view)
	return 0;

    int constraints = theConstraints();

    char *curr_string = search_text->text();
    if (curr_view != inst.prev_view || !inst.prev_string
	|| strcmp(curr_string, inst.prev_string) != 0
	|| constraints != inst.prev_constraints)
    {
	new_search = 1;
    }

    if (new_search && reset)
    {
	inst.prev_view = curr_view;
	inst.prev_constraints = constraints;
	if (inst.prev_string)
	    free(inst.prev_string);
	inst.prev_string = strdup(curr_string);
    }

    return new_search;
}

int Quick_Search::set_widgets()
{
    Initialize(Quick_Search::set_widgets);

    int new_search = new_search_needed(0);
    char *old_title = shell->button("next")->title();
    if (new_search) {
	if (strcmp(TXT("First"), old_title)){
	    shell->button("next")->title(TXT("First"));
	    shell->button("prev")->title(TXT("Last"));
	}
    } else {
	if (strcmp(TXT("Next"), old_title)){
	    shell->button("next")->title(TXT("Next"));
	    shell->button("prev")->title(TXT("Previous"));
	}
    }

    boolean enable_dialog = (curr_view && (!is_steRawView(curr_view)));

    if (last_enable_dialog != enable_dialog){
	((gtPrimitive*)shell->container())->set_sensitive(enable_dialog);
	shell->button("next")->set_sensitive(enable_dialog);
	shell->button("prev")->set_sensitive(enable_dialog);
        last_enable_dialog = enable_dialog;
    }

    if (enable_dialog) {
	ldrPtr ldr_header =
	    curr_view ? curr_view->get_ldrHeader() : NULL;
	ldrTreePtr ldr_root =
	    ldr_header ? checked_cast(ldrTree, ldr_header->get_root()) : NULL;
	appTreePtr app_tree =
	    ldr_root ? ldr_root->get_appTree() : NULL;
        boolean attributes_setting = 1; //(app_tree && is_smtTree(app_tree));

        if (last_attributes_setting != attributes_setting) {
            for (int i=0; i<NUM_CONSTRAINT_BUTTONS; i++)
                    tbox2->button(button_info[i].name)->
                        set_sensitive(attributes_setting);
            last_attributes_setting = attributes_setting;
        }
    }
    return new_search;
}

void Quick_Search::view_changed()
{
    if (curr_view) {
	((gtPrimitive*)shell->container())->set_sensitive(1);
	shell->button("next")->set_sensitive(1);
	shell->button("prev")->set_sensitive(1);
	set_widgets();
    } else {
	((gtPrimitive*)shell->container())->set_sensitive(0);
	shell->button("next")->set_sensitive(0);
	shell->button("prev")->set_sensitive(0);
    }
}

extern void cmd_report_last_count (int);

void Quick_Search::set_match_count (int cnt)
{
    Initialize(Quick_Search::set_match_count);
    inst.match_count = cnt; 
    cmd_report_last_count (cnt);
}

void Quick_Search::init_search()
{
    int constraints = theConstraints();

    char *curr_string = search_text->text();
    search = new Search(curr_view, curr_string, constraints);
    set_match_count (search->matches());

    if (inst.match_count < 1)
	inst.prev_view = NULL;

    char buffer[30];
    OSapi_sprintf(buffer,"%d", inst.match_count);
    matches->text(buffer);
    inst.current_count = inst.match_count;
}

void Quick_Search::do_it(int forw)
{
    Initialize(Quick_Search::do_it);
    push_busy_cursor();
    if (!curr_view)
	return;
    int new_search = new_search_needed(1);
    if (new_search)
	init_search();
    set_widgets();

    int old_count = inst.current_count;
    if (inst.match_count > 0) {
	if (new_search)	{
	    if (forw)
		inst.current_count = 1;
	    else
		inst.current_count = inst.match_count;
	    search->select(inst.current_count);
	} else {
	    if (forw) {
		if (inst.current_count < inst.match_count) {
		    inst.current_count++;
		    search->select(inst.current_count);
		} else {
		    matches_label->text(TXT("No next match."));
		    matches->text(TXT(""));
		}
	    } else {
		if (inst.current_count > 1) {
		    inst.current_count--;
		    search->select(inst.current_count);
		} else {
		    matches_label->text(TXT("No previous match."));
		    matches->text(TXT(""));
		}
	    }
	}
    } else {
	matches_label->text(TXT("No matches found."));
	matches->text(TXT(""));
    }
    if(old_count != inst.current_count){
	char buffer[30];
	OSapi_sprintf(buffer,"%d of %d", inst.current_count, inst.match_count);
	matches->text(buffer);
	matches_label->text(TXT("Occurrences Found: "));
    }
    pop_cursor();
}

void Quick_Search::Next_CB(gtPushButton *, gtEvent *, void *cd, gtReason)
{
    Initialize(Quick_Search::Next_CB);

    Quick_Search_Ptr NAL = Quick_Search_Ptr(cd);
    NAL->do_it(1);
}
    
void Quick_Search::Previous_CB(gtPushButton *, gtEvent *, void *cd, gtReason)
{
    Initialize(Quick_Search::Previous_CB);

    Quick_Search_Ptr NAL = Quick_Search_Ptr(cd);
    NAL->do_it(0);
}
    
void Quick_Search::Done_CB(gtPushButton *, gtEvent *, void *cd, gtReason)
{
    Initialize(Quick_Search::Done_CB);

    Quick_Search_Ptr NAL = Quick_Search_Ptr(cd);
    delete NAL;
}

void Quick_Search::Constraint_CB(gtToggleButton *tb, gtEvent *, void *cd, gtReason)
{
    Initialize(Quick_Search::Constraint_CB);

    int i;
    Quick_Search_Ptr NAL = Quick_Search_Ptr(cd);
    if (!strcmp(tb->name(), button_info[0].name)){
	tb->set(1, 0);
	for (i=1; i<NUM_CONSTRAINT_BUTTONS; i++)
	    NAL->tbox2->button(button_info[i].name)->set(0, 0);
    } else {
	int one_set = 0;
	for (i=1; i<NUM_CONSTRAINT_BUTTONS; i++)
	    if (NAL->tbox2->button(button_info[i].name)->set()){
		one_set = 1;
		break;
	    }
	NAL->tbox2->button(button_info[0].name)->set(!one_set, 0);
    }
    NAL->set_widgets();
}

void Quick_Search::Case_CB(gtToggleButton *, gtEvent *, void *cd, gtReason)
{
    Initialize(Quick_Search::Case_CB);

    Quick_Search_Ptr NAL = Quick_Search_Ptr(cd);
    NAL->set_widgets();
}

void Quick_Search::Match_CB(gtToggleButton *, gtEvent *, void *cd, gtReason)
{
    Initialize(Quick_Search::Match_CB);

    Quick_Search_Ptr NAL = Quick_Search_Ptr(cd);
    NAL->set_widgets();
}

void Quick_Search::Changed_CB(gtStringEditor *, gtEvent *, void *cd, gtReason)
{
    Initialize(Quick_Search::Changed_CB);

    Quick_Search_Ptr NAL = Quick_Search_Ptr(cd);
    NAL->set_widgets();
}

/*
   START-LOG-------------------------------------------

   $Log: Quick_Search.h.C  $
   Revision 1.8 2000/07/10 23:11:17EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.16  1994/08/01  19:23:58  farber
Bug track: 7411
attach help path

Revision 1.2.1.15  1994/02/16  18:13:07  kws
Port

Revision 1.2.1.14  1993/12/31  19:41:42  boris
Bug track: Test Validation
Test Validation

Revision 1.2.1.13  1993/12/11  16:43:18  jon
Bug track: many
Fixed to work with the new quick search objects.

Revision 1.2.1.12  1993/11/24  20:42:36  azaparov
Fixed bug 5339

Revision 1.2.1.11  1993/08/10  20:14:28  jon
Bug 4186 and 4188

Revision 1.2.1.10  1993/05/26  14:13:47  jon
Fixed next and previous to give a message if there are no more search hits
bug 3447

Revision 1.2.1.9  1993/04/29  18:23:50  jon
Made the quick help dialog disable itself when a raw view is selected
(bug 3031)

Revision 1.2.1.8  1993/02/10  23:03:44  jon
Made assoc constraint button stay sensitive on non-smt views (bug 2395)
Removed need for viewershell parameter in constructor.
Cleaned up.

Revision 1.2.1.7  1993/02/08  16:23:36  glenn
Change decl of apl_SEARCH_create_search.

Revision 1.2.1.6  1993/01/21  15:03:53  jon
Fixed to work with new STE/S-Mode selection mech.

Revision 1.2.1.4  1993/01/14  16:00:23  jon
Pitched bookmarks, back to old mech.

Revision 1.2.1.3  1992/11/12  22:11:49  jon
Disabled Match Word since our regexp dosn't support it yet.
Eliminated extra error message when no matches are found.

Revision 1.2.1.2  1992/10/09  20:04:32  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/

