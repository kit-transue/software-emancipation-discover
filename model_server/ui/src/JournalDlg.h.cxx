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
// JournalDlg.h.C
//------------------------------------------
// synopsis:
// JournalDlg in gtLib.
//
// description:
// Popup a dialog box with the following lines of input:
// Name of Journal:
// Journal Bug_number:
// Date Journal was created:
// pdf used:
// Name of Project:
// along with the usual OK/CANCEL/HELP buttons.
//------------------------------------------

#include <msg.h>
#include "machdep.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */
#include <messages.h>
#include <cmd.h>

#include <gtNoParent.h>
#include <gtDlgTemplate.h>
#include <gtLabel.h>
#include <gtStringEd.h>

#include <genString.h>

#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
#endif /* ISO_CPP_HEADERS */

#include <gtPushButton.h>
#include <JournalDlg.h>
#include <proj.h>
#include <options.h>
#include <evalMetaChars.h>

void project_convert_filename(char *path, genString &ret_path);

static const char *  cmd_journal_subsystem = 0;
static const char *  cmd_journal_hpn = 0;

JournalDlg::JournalDlg(gtBase* parent)
// constructor for the journal dialog box
{
#ifndef NEW_UI
    shell = gtDialogTemplate::create(parent, "New_Journal", "New Journal");
    shell->add_button("ok", TXT("OK"), &JournalDlg::OK_CB, this);
    shell->add_button("cancel", TXT("Cancel"), &JournalDlg::Cancel_CB, this);
    gtPushButton::next_help_context("Pset.Help.Internals.StartJournal");
    shell->add_help_button();
 
    label = gtLabel::create(shell, "Journal_Name", "Journal Name");
    label->attach(gtLeft);
    label->attach(gtRight);
    label->attach(gtTop);
    label->alignment(gtBeginning);
    label->manage();
 
    answer_text = gtStringEditor::create(shell, "answer", NULL);
    answer_text->attach(gtTop, label);
    answer_text->attach(gtLeft);
    answer_text->attach(gtRight);
    answer_text->manage();
    
#if defined(USE_PDF_DIR_FOR_JOURNAL)
    char * dir = get_main_option("-P");

    if(!dir)
        dir = OSapi_getenv("PSET_PROJECT_FILE");
#endif    // USE_PDF_DIR_FOR_JOURNAL

    // bug13016 - Change to always start with current dir.
    char* dir = OSapi_getenv("PWD");

    genString path;
    genString bug_number;
    if (dir) {
        if (dir[0] != '/')
            path.printf("%s/%s", OSapi_getenv("PWD"), dir);
        else
            path = dir;
        dir = path;

        genString ddd;
        project_convert_filename(dir, ddd);
        dir = ddd;

        char * slash = strrchr(dir, '/');
        if(!slash){
          dir = 0;
        } else {
#if defined(USE_PDF_DIR_FOR_JOURNAL)
          path.put_value(dir, slash-dir);
#endif    // USE_PDF_DIR_FOR_JOURNAL
          const char* bugname = strrchr(path.str(), '/');
	  if (bugname == NULL)
	      bugname = path.str();
	  if (bugname != NULL) {
              while (bugname[0] && (bugname[0] < '0' || bugname[0] > '9'))
		  ++bugname;
              int bug = bugname[0] ? atoi(bugname) : 0;
              if (bug > 0) 
		  bug_number.printf("%d", bug);
	  }
        }
    }
    path += "/test.ccc";
    answer_text->text((char*)path);

    bug_number_label = gtLabel::create(shell, "bug_number", "Bug Number");
    bug_number_label->attach(gtLeft);
    bug_number_label->attach(gtRight);
    bug_number_label->attach(gtTop, answer_text, 10);
    bug_number_label->alignment(gtBeginning);
    bug_number_label->manage();

    bug_number_text = gtStringEditor::create(shell, "bug_text", NULL);
    bug_number_text->attach(gtLeft);
    bug_number_text->attach(gtRight);
    bug_number_text->attach(gtTop, bug_number_label);
    bug_number_text->manage();
    bug_number_text->text((char*) bug_number);

    int sz;
    genEnum_item* arr;
    CMD_VALIDATION_GROUP_name_data(&arr, &sz);
 

    subsystem = gtOptionMenu::create(shell, "subsystem", "Validate:", NULL);
    for(int ii=0; ii<sz; ++ii){
      const char * nm = arr[ii].idx_name + 4;
      subsystem->insert_entries(-1, gtMenuStandard, nm, nm, nm, 
		      JournalDlg::subsystem_CB, NULL);
    }

    subsystem->attach(gtLeft);
    subsystem->attach(gtRight);
    subsystem->attach(gtTop, bug_number_text, 10);
    subsystem->manage();
    subsystem->menu_history(0);

    projNode* home = projNode::get_home_proj();
    cmd_journal_hpn = home->get_name();
    genString hp;
    hp.printf("Home Project: %s", cmd_journal_hpn);


    project_used = gtLabel::create(shell, "project_name", (char*)hp);
    project_used->attach(gtLeft);
    project_used->attach(gtRight);
    project_used->attach(gtTop, subsystem);
    project_used->alignment(gtBeginning);
    project_used->manage();

#endif //NEW_UI
}
 
JournalDlg::~JournalDlg()
{
    if (shell) delete shell;
}

int JournalDlg::oneshot(genString& answer, genString& bug_number, genString& 
			subsys, genString& /*pdf_name*/, genString& project_name)
{
    if ((const char*)answer)
    	answer_text->text(answer);
    if ((const char*)bug_number)
	bug_number_text->text(bug_number);

    confirm = 0;
#ifndef NEW_UI
    shell->popup(3);
    shell->take_control(&JournalDlg::take_control_CB, this);
    shell->popdown();

    if (confirm > 0) {
	answer = answer_text->text();
	bug_number = bug_number_text->text();
	subsys = cmd_journal_subsystem;
	project_name =cmd_journal_hpn;
    }
    cmd_journal_subsystem = 0;
    cmd_journal_hpn = 0;
#endif
    return confirm;
}

void JournalDlg::OK_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    JournalDlg_Ptr pp = JournalDlg_Ptr(cd);

    char * fn = pp->answer_text->text();
    genString journalName = fn;

     // expand the value.
     if (journalName.length() > 0) {
         if (eval_shell_metachars(fn, journalName) ) {
            pp->answer_text->text((char*)journalName);
         } else {
            msg("Problem expanding value \'$1\' for journal name \'$2\'", error_sev) << (char*) journalName << eoarg << fn << eom;
            journalName = "";
        }
    }

    // check for an empty filename
    if ( journalName.length() > 0 ) {
        // check for a invalid filename
        ofstream file_stream((char*)journalName, ios::out);
        if (file_stream) {
            // Valid journal name given.
            file_stream.close();
            pp->confirm = 1;
            pp->shell->popdown();
        } else {
            // Invalid journal name given.
            msg("$1: Invalid filename", error_sev) << (char*)journalName << eom;
        }

    } else { 
        // Empty journal name given.
	msg("No filename given", error_sev) << eom;
    }
#endif
}

void JournalDlg::Cancel_CB(gtPushButton *, gtEventPtr, void *cd, gtReason)
{
#ifndef NEW_UI
    JournalDlg_Ptr pp = JournalDlg_Ptr(cd);

    pp->confirm = -1;
    pp->shell->popdown();
#endif
}

int JournalDlg::take_control_CB(void *cd)
{
#ifndef NEW_UI
    return ((JournalDlg_Ptr)cd)->confirm;
#else
    return 0;
#endif
}

//////////////////////////////////////////////////////////////////////////
// C function to replace old JournalDlg.if interface
//////////////////////////////////////////////////////////////////////////

#include <gtBaseXm.h>

extern "C" int JournalDlg_oneshot(Widget* wp, char **answer, char **bn,
				  char **gr, char **pdf, char **proj)
{
#ifndef NEW_UI
    gtNoParent* parent = wp ? gtNoParent::create() : NULL;
    if(parent)
	parent->rep()->widget(*wp);

    JournalDlg pr(parent);

    genString string(answer ? *answer : NULL);
    genString bug_number(bn ? *bn : NULL);
    genString create_date(gr ? *gr : NULL);
    genString pdf_name(pdf ? *pdf : NULL);
    genString proj_name(proj ? *proj : NULL);

    const int confirm = pr.oneshot(string, bug_number, create_date, pdf_name,
				   proj_name);

    *answer = NULL;
    *bn = NULL;
    *gr = NULL;
    *pdf = NULL;
    *proj = NULL;

    if(confirm > 0  &&  (const char*)string) {
	*answer = strdup(string);
	if ((const char*)bn) *bn = strdup(bug_number);
	if ((const char*)pdf) *pdf = strdup(pdf_name);
	if ((const char*)proj) *proj = strdup(proj_name);
    }

    if(parent)
    {
	parent->rep()->widget(NULL);
	delete parent;
    }

    return confirm;
#else
    return 0;
#endif
}

void JournalDlg::subsystem_CB(void*, gtEventPtr, void* data, gtReason){
#ifndef NEW_UI
  cmd_journal_subsystem = (char*) data;
#endif
 }

/*
   START-LOG-------------------------------------------

   $Log: JournalDlg.h.C  $
   Revision 1.16 2000/11/30 09:55:42EST sschmidt 
   Fix for bug 20091: improve output during crashes, sensitivity of tests
// Revision 1.2  1994/03/06  16:36:12  mg
// Bug track: 0
// validation groups
//
// Revision 1.1  1993/09/21  21:57:48  andrea
// Initial revision
// 

  END-LOG---------------------------------------------
*/

