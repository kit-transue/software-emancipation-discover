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
#if 0 //#ifndef _RTL_apl_extern_h
#define _RTL_apl_extern_h

#ifndef _dialog_h
#include <dialog.h>
#endif
#ifndef _prototypes_h
#include <prototypes.h>
#endif
#ifndef _RTL_Names_h
#include <RTL_Names.h>
#endif
#ifndef _RTL_externs_h
#include <RTL_externs.h>
#endif

#ifdef PROTOTYPES

#ifdef __cplusplus

   extern "C" {

      RTLNode *apl_RTL_get_rtl (char *rtl_name);

      Obj *apl_RTL_find_obj (RTLNode *rtl, char *name);

      Obj *apl_RTL_Get_Obj_At_Index (dialog *dlg, int view_num, int index);

      void apl_RTL_delete_rtl (char *rtl_name);

      RTLNode *apl_RTL_merge_rtl (char *dest_rtl, char *rtl);

      RTLNode *apl_RTL_intersect_rtl (char *dest_rtl, char *rtl);

      RTLNode *apl_RTL_subtract_rtl (char *dest_rtl,  char *rtl);

      void apl_RTL_merge_selected_rtls (dialog *dlg, char *rtl_name);

      void apl_RTL_subtract_selected_rtls
	(dialog *dlg, char *rtl_name, char *from_rtl);

      void apl_RTL_intersect_selected_rtls (dialog *dlg, char *rtl_name);

      void apl_RTL_save (char *rtl_name, int contents, int search);

      RTLNode *apl_RTL_create (char *rtl_name);

      void  apl_RTL_add_preview (char *rtl_name, char *preview);

      char *apl_RTL_get_preview (char *rtl_name);

      void apl_RTL_add_item (char *rtl_name, Obj *obj);

      void apl_RTL_add_cur_selection (char *rtl_name);

      void apl_RTL_delete_item (char *rtl_name, Obj *obj);

      void apl_RTL_remove_selected_rtl (dialog *dlg, char *rtl_name);

      void apl_RTL_remove_selected (dialog *dlg, char *rtl_name);

      void apl_RTL_delete_all (char *rtl_name);

      void apl_RTL_add_selection_obj (char *rtl_name, Obj *obj);

      void apl_RTL_remove_selection_obj (char *rtl_name, Obj *obj);

//      objArr *apl_RTL_get_selection (char *rtl_name);

      void apl_RTL_set_selection (char *rtl_name, objArr *selection);

      void apl_RTL_header_open_dialog (RTL *rtl_head,
				void *(fn) (dialog *, void *),
				void *swidget_parent,
				repType view_type);

      void apl_RTL_new_open_dialog (RTL *rtl_head,
				void *(fn) (dialog *, void *),
				void *swidget_parent,
				repType view_type,
				int sort_order,
				ldrRTLDisplayType  fields);

      void apl_RTL_open_dialog (char *rtl_name,
				void *(fn) (dialog *, void *),
				void *swidget_parent,
				repType view_type,
                                int sort_order,
                                ldrRTLDisplayType  fields);

      void apl_RTL_open_selected_rtls (dialog *dlg,
				       void *(fn) (dialog *, void *),
				       void *swidget_parent);

      void apl_RTL_operate_on_selected (dialog *dlg,
                                void *(fn) (Obj *obj));

      RTLNode *apl_RTL_create_RTL_of_RTLS (char *rtl_name);

      void apl_RTL_open_view
	(dialog *dlg, int view_num, char *rtl_name,
	 repType view_type, int sorting_order,
	 ldrRTLDisplayType d_type);

      void apl_RTL_change_view_to_text (dialog *dlg, int view_num,
					char *rtl_name);

      void apl_RTL_change_view_to_diag (dialog *dlg, int view_num,
					char *rtl_name);

      int apl_RTL_includes (Obj *obj, char *rtl_name);

      void apl_RTL_add_new_filter
	(char *rtl_name, FilterDomainType filter_type, char *value);

      int apl_RTL_get_filter_toggle (char *rtl_name);

      void apl_RTL_filter_toggle (char *rtl_name, int on);

      void apl_RTL_clear_filter (char *rtl_name);

      void apl_RTL_make_udl (char *rtl_name);

      int apl_RTL_is_UDL (char *rtl_name);
   }

#endif /* __cplusplus */

#endif /* PROTOTYPES */

/*
    START-LOG-------------------------------

    $Log: RTL_apl_extern.h  $
    Revision 1.1 1993/05/27 16:22:53EDT builder 
    made from unix file
 * Revision 1.2.1.4  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.3  1993/04/20  15:43:02  davea
 * bug 3437: remove prototype for obsolete fct apl_RTL_get
 *
 * Revision 1.2.1.2  1992/10/09  19:56:04  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

#endif /* _RTL_apl_extern_h */
