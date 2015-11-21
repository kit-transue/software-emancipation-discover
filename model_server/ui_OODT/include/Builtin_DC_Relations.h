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
#ifndef _Builtin_DC_Relations_h
#define _Builtin_DC_Relations_h

// Builtin_DC_Relations.h
//------------------------------------------
// synopsis:
// Dialog for setting builtin-relation filter for Data Charts
//------------------------------------------

#ifndef _prototypes_h
#include <prototypes.h>
#endif
#ifndef _gtDlgTemplate_h
#include <gtDlgTemplate.h>
#endif
#ifndef _gtToggleBox_h
#include <gtToggleBox.h>
#endif
#ifndef _oodt_ui_decls_h
#include <oodt_ui_decls.h>
#endif
#ifndef _objRelation_h
#include <objRelation.h>
#endif


RelClass(view);

BEGIN_C_DECL
void popup_Builtin_DC_Relations PROTO((viewPtr));
END_C_DECL

class Builtin_DC_Relations {
    gtToggleBox		*togbox;
    viewPtr		my_viewp;

  public:
    gtDialogTemplate 	*shell;
    
    Builtin_DC_Relations();
    void init(viewPtr);
    void clear_toggles();
    void do_it();
    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
};

typedef Builtin_DC_Relations* Builtin_DC_Relations_Ptr;


/*
    START-LOG-------------------------------

    $Log: Builtin_DC_Relations.h  $
    Revision 1.1 1993/05/27 16:22:41EDT builder 
    made from unix file
// Revision 1.2  1993/05/27  20:15:41  glenn
// NewFileSelector.h Noun_Context.h Noun_Search.h Nouns_RTL.h OODT_apl_entries.h PROJ_View_Pref.h Panner.h Print.h ProcessSelector.h Quick_Search.h RTL_apl_extern.h RTL_filter.h STE_Create_sd.h STE_Edit_Style.h STE_Edit_Style_Doc.h STE_Edit_Style_RGB.h STE_Edit_Style_TP.h SelectFname.h SelectFromList.h SoftAssoc_apl.h Text_Field.h all-interns.h all-stubs.h browserShell.h button_bar.h change_propagator.h customize.h dataDefn.h errorReport.h extract-dialog.h fileLanguageDlg.h generic-popup-menu.h help.h impactAnalysis.h oodt_class_member_sorter.h oodt_relation.h propagator_apl.h quickHelp.h stackmgr.h systemMessages.h top_widgets.h typeIn_Propagator.h ui-globals.h ui-rowcolumn.h uimx_context.h waiter.h waiter_types.h watchVar.h x11_intrinsic.h
//
// Revision 1.1  1992/12/17  21:36:09  wmm
// Initial revision
//
    END-LOG---------------------------------
*/

#endif // _Builtin_DC_Relations_h
