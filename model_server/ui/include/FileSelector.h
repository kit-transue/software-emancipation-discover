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
#ifndef _FileSelector_h
#define _FileSelector_h

#if 0
// FileSelector.h
//------------------------------------------
// synopsis:
// Declaration of standard file-selector interface
//------------------------------------------
// Restrictions:
// This file must be C compatible
//------------------------------------------
#endif

#ifndef _prototypes_h
#include <prototypes.h>
#endif

/* If you need <X11/Intrinsic.h>, include it *before* this file. */
#ifndef _XtIntrinsic_h
typedef struct _WidgetRec *Widget;
#endif

BEGIN_C_DECL


/* FileSelector -- Select from existing files */

Widget create_FileSelector PROTO((Widget parent));
void   init_FileSelector PROTO((
		Widget, char* title, char* prompt, char* filetype,
		void (*callback)(Widget, char* filename, char* filetype) ));
void   init_FileSelector_w_Args PROTO((
		Widget, char* title,
		void (*callback)(Widget, char* filename, char* arguments) ));
void   setargs_FileSelector PROTO((Widget, char *arguments));
void   popup_FileSelector PROTO((Widget));
void   popdown_FileSelector PROTO((Widget));
void   destroy_FileSelector PROTO((Widget));


END_C_DECL


/*
    START-LOG-------------------------------

    $Log: FileSelector.h  $
    Revision 1.1 1993/05/27 16:22:45EDT builder 
    made from unix file
 * Revision 1.2.1.3  1993/05/27  20:15:41  glenn
 * NewFileSelector.h Noun_Context.h Noun_Search.h Nouns_RTL.h OODT_apl_entries.h PROJ_View_Pref.h Panner.h Print.h ProcessSelector.h Quick_Search.h RTL_apl_extern.h RTL_filter.h STE_Create_sd.h STE_Edit_Style.h STE_Edit_Style_Doc.h STE_Edit_Style_RGB.h STE_Edit_Style_TP.h SelectFname.h SelectFromList.h SoftAssoc_apl.h Text_Field.h all-interns.h all-stubs.h browserShell.h button_bar.h change_propagator.h customize.h dataDefn.h errorReport.h extract-dialog.h fileLanguageDlg.h generic-popup-menu.h help.h impactAnalysis.h oodt_class_member_sorter.h oodt_relation.h propagator_apl.h quickHelp.h stackmgr.h systemMessages.h top_widgets.h typeIn_Propagator.h ui-globals.h ui-rowcolumn.h uimx_context.h waiter.h waiter_types.h watchVar.h x11_intrinsic.h
 *
 * Revision 1.2.1.2  1992/10/09  19:55:34  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif /* _FileSelector_h */
