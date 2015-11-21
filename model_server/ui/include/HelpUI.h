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
#ifndef _HelpUI_h
#define _HelpUI_h

// HelpUI.h
//------------------------------------------
// synopsis:
// UI for help buttons
//
//------------------------------------------

#ifndef _steDocument_h
#include <steDocument.h>
#endif

#ifndef _gt_h
#include <gt.h>
#endif

#ifndef _genArr_h
#include <genArr.h>
#endif

#define HELP_INDEX_NAME "/ParaDOCS/help_context_links.txt"

typedef gtBase *gtBasePtr;
genArr(gtBasePtr);

extern "C" void HelpUI_HelpWidget_CB(Widget w, void*);
extern void HelpUI_LongHelp(gtBase* base);
extern void HelpUI_LoadWidgetIndex();

class HelpUI
{
  public:
    static HelpUI* instance();

    static void dump_widget_index();
    static void edit_widget_index();

    static void set_show_pathnames(boolean);
    static boolean get_show_pathnames();
    
    static void set_use_helper_shell(boolean);
    static boolean get_use_helper_shell();

    boolean LoadWidgetIndex();
    boolean WidgetIndexAvailable(boolean repeat_message);
    commonTreePtr IndexRoot(boolean repeat_message);

    void OpenSection(const char *path);
  
  private:
    static HelpUI* the_instance;

    steDocumentPtr widget_index;
    boolean already_tried_it;
    boolean show_pathnames;
    boolean use_helper_shell;

    HelpUI();
    ~HelpUI();
};

void HelpUI_LongHelp(gtBase*);
void HelpUI_LoadWidgetIndex();

extern "C" void HelpUI_HelpWidget_CB(Widget, void*);

class HelpHistory
{
  public:
    HelpHistory(gtBase* parent, int old_view_deleted);
    ~HelpHistory();
    gtDialogTemplate* get_shell() { return shell; }
    int build_interface(gtBase* parent);
    void apply();

    static void pull_down();
    static HelpHistory *load_HelpHistory(int old_view_deleted);

    static void his_select_CB (gtList*, gtEventPtr, void* data, gtReason);
    static void his_action_CB (gtList*, gtEventPtr, void* data, gtReason);

  private:
    static HelpHistory *help_history_obj;

    gtDialogTemplate* shell;
    gtLabel* history_text_label;
    gtRTL* history_text_rtl;
    int ov_deleted;

    static void ok_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void apply_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
    static void cancel_CB (gtPushButton*, gtEventPtr, void* data, gtReason);
//  static void his_select_CB (gtList*, gtEventPtr, void* data, gtReason); // made public
//  static void his_action_CB (gtList*, gtEventPtr, void* data, gtReason); // made public

    RTLNode *history_text_nodes;
};

/*
    START-LOG-------------------------------

    $Log: HelpUI.h  $
    Revision 1.6 1996/03/06 21:50:20EST abover 
    sun4 SW port
 * Revision 1.2.1.8  1994/07/27  00:05:23  jethran
 * removed quickhelp stuff as part of overall removal of quickhelp
 *
 * Revision 1.2.1.7  1993/05/27  20:15:41  glenn
 * NewFileSelector.h Noun_Context.h Noun_Search.h Nouns_RTL.h OODT_apl_entries.h PROJ_View_Pref.h Panner.h Print.h ProcessSelector.h Quick_Search.h RTL_apl_extern.h RTL_filter.h STE_Create_sd.h STE_Edit_Style.h STE_Edit_Style_Doc.h STE_Edit_Style_RGB.h STE_Edit_Style_TP.h SelectFname.h SelectFromList.h SoftAssoc_apl.h Text_Field.h all-interns.h all-stubs.h browserShell.h button_bar.h change_propagator.h customize.h dataDefn.h errorReport.h extract-dialog.h fileLanguageDlg.h generic-popup-menu.h help.h impactAnalysis.h oodt_class_member_sorter.h oodt_relation.h propagator_apl.h quickHelp.h stackmgr.h systemMessages.h top_widgets.h typeIn_Propagator.h ui-globals.h ui-rowcolumn.h uimx_context.h waiter.h waiter_types.h watchVar.h x11_intrinsic.h
 *
 * Revision 1.2.1.6  1993/02/03  16:28:49  jon
 * Changed index name again.
 *
 * Revision 1.2.1.5  1993/01/25  20:53:34  jon
 * Updated help and quick help.
 *
 * Revision 1.2.1.4  1992/12/12  07:09:57  glenn
 * New view creation interface.
 *
 * Revision 1.2.1.3  1992/11/05  18:10:50  jon
 * added get_show_pathnames() and set_show_pathnames
 *
 * Revision 1.2.1.2  92/10/09  19:55:39  kws
 * Fix comments
 * 
    END-LOG---------------------------------
*/

#endif // _HelpUI_h

