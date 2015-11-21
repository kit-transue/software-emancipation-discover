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
// ste_style_handler.h
//------------------------------------------
#ifndef ste_style_handler_h
#define ste_style_handler_h

#include "objRelation.h"
#include "objOper.h"
#include "steFontHandler.h"
#include "steEpochStyle.h"
#include "steStyle.h"
#include "steAppStyle.h"
#include "steView.h"

RelClass(steDocument);

define_relation(default_style_of_ste,ste_of_default_style);
define_relation(hilite_style_of_ste,ste_of_hilite_style);
define_relation(default_style_of_view,view_of_default_style);
define_relation(hilite_style_of_view,view_of_hilite_style);


extern void ste_propagate_style_apply( appTreePtr );
extern steAppStylePtr get_global_style_table();
extern steAppStylePtr get_default_app_style();
extern steAppStylePtr get_create_app_style( appPtr );
extern steAppStylePtr get_default_app_style( int app_type );
extern steSlotPtr get_node_character_style( commonTreePtr,
                                            steViewPtr v = NULL );
extern steSlotPtr get_node_style(commonTreePtr node);
extern steAppStylePtr get_node_style_table(commonTreePtr node);
extern steFontHandlerPtr get_font_handler( steEpochStylePtr );

extern void ste_assign_style(steSlotPtr);
extern void ste_deassign_style(steSlotPtr);
extern steSlotPtr get_style_by_name(const char *nm);
extern steSlotPtr get_style_by_name(const char *nm, appTreePtr nd);
extern steSlotPtr get_style_by_name(const char *nm, viewPtr);
extern steSlotPtr get_style_by_name(const char *nm, appPtr);
extern void ste_delete_style( steSlotPtr );
extern steSlotPtr ste_define_style( steStylePtr );
extern steViewPtr get_epochstyle_view(steEpochStylePtr);
extern int get_epochstyle_screen_id(steEpochStylePtr);
extern int ste_is_splice_node_style(commonTreePtr);
extern int ste_get_smt_title_type(commonTreePtr);
extern int ste_get_smt_clause_style(commonTreePtr);
extern int ste_is_base_style_type ( ste_style_type tp );
extern int ste_get_smt_comment_type( commonTreePtr );
extern int ste_get_empty_line_type ( commonTreePtr );
extern int ste_get_node_style_type ( commonTreePtr );
extern void ste_smt_format_values( steDisplayNodePtr, 
                 int *left, int *lines_v, int *indent_v);
extern ste_set_reporter_styles(steDocument *);
#endif

/*
   START-LOG-------------------------------------------

   $Log: ste_style_handler.h  $
   Revision 1.2 1994/08/30 15:38:20EDT boris 
   Bug track: n/a
   Added changable style for ParaREPORT
 * Revision 1.3  1993/04/20  15:58:40  boris
 * Fixed bug #3377. With deassign styles/categories
 *
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.2  92/10/09  20:01:50  boris
 * Fix comment
 * 


   END-LOG---------------------------------------------

*/
