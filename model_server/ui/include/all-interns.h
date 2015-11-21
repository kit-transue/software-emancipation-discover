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
#ifndef _all_interns_h
#define _all_interns_h

/*  all-interns.h -  note, this include file must be usable from either C or C++ files  */

#ifndef _prototypes_h
#include <prototypes.h>
#endif

BEGIN_C_DECL

swidget  popup_creator			PROTO((  char *what, struct ui_list *(*getfn)(void), 
					 swidget (*editfn)(char *, swidget),
					 void (*createfn)(char*,char*), swidget parent ));
swidget create_db_selector	 	PROTO((  struct ui_selector_info *info, swidget parent ));
swidget  popup_display_list		PROTO((  struct ui_list *l, char *win_title, swidget parent ));
swidget create_file_selector		PROTO((  struct ui_selector_info *info, swidget parent ));
swidget  popup_generic_error		PROTO((  int x, int y, char *msg, swidget parent ));
swidget  popup_generic_list		PROTO((  struct ui_list *l, char *win_title, int ismultiple, 
					 void (*call)(struct ui_list *), swidget parent ));
swidget  popup_generic_message		PROTO((  int x, int y, char *title, char *msg, swidget parent ));
swidget  popup_generic_text2		PROTO((  char *win_title, char *init_text, void (*call)(char *), 
					 swidget parent ));
swidget  popup_help			PROTO((  char *title, char *text, char *path, swidget parent ));
swidget  popup_oneliner			PROTO((  char *win_title, char *init_text, void (*call)(char *), 
					 swidget parent ));
swidget  popup_oneliner_withMenu	PROTO((  char *win_title, char *init_text, 
					 struct ui_list *(*getlist)(void), void (*call)(char *), 
					 swidget parent ));
swidget  popup_Replace			PROTO((  void (*func)(char*, char*, int, int), swidget parent ));
swidget  popup_rtl_shell2		PROTO((  struct ui_list *l, int x, int y, void (*func)(char *), 
					 swidget parent ));
swidget  popup_Search			PROTO((  void (*func)(char *, int), swidget parent ));
swidget  popup_PROJECT			PROTO((  void ));
swidget  popup_STE			PROTO((  void *dialog, swidget *parent ));
swidget  popup_Edit_Category		PROTO((  char *name, swidget parent ));
swidget  popup_Edit_Style		PROTO((  char *name, swidget parent ));
swidget  popup_Edit_Format		PROTO((  char *name, swidget parent ));
swidget  popup_Edit_NumTbl		PROTO((  char *name, swidget parent ));
swidget  popup_make_struct_doc		PROTO((  swidget parent ));
swidget  popup_make_new_node		PROTO((  swidget parent ));
swidget  popup_SPD			PROTO((  void *dialog, swidget *parent ));
swidget  popup_Insert_Block		PROTO((  swidget parent ));
swidget  popup_Insert_Case		PROTO((  swidget parent ));
swidget  popup_Insert_FunctionCall	PROTO((  swidget parent ));
swidget  popup_Insert_If		PROTO((  swidget parent ));
swidget  popup_Insert_Statement		PROTO((  swidget parent ));
swidget  popup_Insert_Variable		PROTO((  void *dialog, swidget parent ));
swidget  popup_Insert_Loop		PROTO((  swidget parent ));
swidget create_Insert_Loop_Cond		PROTO((  struct ui_list *conditions, swidget button ));
swidget create_Insert_Loop_For		PROTO((  struct ui_list *varList, struct ui_list *startList, 
					 struct ui_list *endList, struct ui_list *incrList,
					 swidget button ));
swidget create_Insert_Loop_Init		PROTO((  struct ui_list *nameList, struct ui_list *typeList, 
					 struct ui_list *valueList, swidget button ));
swidget  popup_Run_Eval			PROTO((  swidget parent ));
swidget  popup_Define_Data_Array	PROTO((  char *name, swidget parent ));
swidget  popup_Define_Data_Char		PROTO((  char *name, swidget parent ));
swidget  popup_Define_Data_Comp		PROTO((  char *name, swidget parent ));
swidget  popup_Define_Data_Int		PROTO((  char *name, swidget parent ));
swidget  popup_Define_Data_Real		PROTO((  char *name, swidget parent ));
swidget  popup_Define_Data_Record	PROTO((  char *name, swidget parent ));
swidget create_domain_shell		PROTO((  char *name, swidget parent ));
swidget  popup_Debug_TopLevel		PROTO((  swidget parent ));
swidget  popup_Debug_BreakCond		PROTO((  void ));
swidget  popup_Debug_Variable		PROTO((  void ));



  void *ui_declare_regexp    PROTO((  char *mask ));
  int ui_match_regexp        PROTO((  void *p, char *name ));
  void ui_finish_with_regexp PROTO((  void *p ));

END_C_DECL


/*
    START-LOG-------------------------------

    $Log: all-interns.h  $
    Revision 1.1 1993/05/27 16:23:01EDT builder 
    made from unix file
 * Revision 1.2.1.4  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.3  1993/04/28  00:23:16  davea
 * bug 3481 - cleaned up #ifdef/paste hack, replacing with PROTO(())
 *
 * Revision 1.2.1.2  1992/10/09  19:56:30  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif
