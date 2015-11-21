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
/////////////////////////   FILE ste_category_handler.h   ////////////////////
//
// -- Contains functions for handling categfories. 
//
//         boolean root_category_test( commonTreePtr, steCategoryPtr);
//
#ifndef ste_category_handler_h
#define ste_category_handler_h

#include "ste.h"
#include "steSlot.h"
#include "objOper.h"

RelClass(steView);
RelClass(steDisplayNode);
RelClass(steCategory);
RelClass(steDocument);
RelClass(steTextNode);
RelClass(app);
RelClass(steAppCategory);

extern steSlotPtr  get_category_by_name(const char* nm);
extern steSlotPtr  get_category_by_name(const char* nm, appTreePtr);
extern steSlotPtr  get_category_by_name(const char* nm, appPtr);
extern steSlotPtr  get_category_by_name(const char* nm, viewPtr);
extern steSlotPtr  get_category(int nm);
extern steSlotPtr  get_category(int nm, appTreePtr);
extern steSlotPtr  get_category(int nm, appPtr);
extern steSlotPtr  get_category(int nm, viewPtr);
extern void select_cat(steTextNodePtr  root, steSlotPtr my_cat);
extern void ste_select_category( steSlotPtr );
extern void ste_export_ring(steSlotPtr categ, steDocumentPtr from_doc,
                            steDocumentPtr targ_doc);
extern void ste_direct_export_ring( steDocumentPtr nld_doc );
extern void ste_export_category( steDocumentPtr targ_doc, steSlotPtr cat );
extern void ste_insert_into_active_category();
extern steTextNodePtr ste_get_item_header( steTextNodePtr txt );
extern void ste_remove_category_item( steTextNodePtr w_txt );
extern void remove_category_from_node( appTreePtr txt, steSlotPtr cat );
extern void remove_category_from_root( appTreePtr txt, steSlotPtr cat );
extern void ste_assign_category( steSlotPtr categ );
extern void ste_deassign_category( steSlotPtr categ );

extern void ste_repeat_assign_category(appTreePtr src, appTreePtr trg);

extern steAppCategoryPtr get_global_system_category_table();
extern steAppCategoryPtr get_global_user_category_table();
extern steAppCategoryPtr get_app_system_category_table();
extern steAppCategoryPtr get_app_user_category_table();
extern steAppCategoryPtr get_create_local_system_category(appPtr);
extern steAppCategoryPtr get_create_local_user_category(appPtr);

extern boolean root_category_test(commonTreePtr, steSlotPtr);
extern void ste_add_category(appTreePtr node, steSlotPtr categ);
extern Obj *get_node_category(commonTreePtr);
extern void ste_get_node_default_categories(commonTreePtr, bool is_parsed,
				       objArr&);
extern void ste_warn_about_default_category_conflicts(steSlotPtr cat_slot,
						      steSlotPtr style_slot);
#endif

/*
   START-LOG-------------------------------------------

   $Log: ste_category_handler.h  $
   Revision 1.2 1994/11/07 11:29:50EST zdenek 
   gcc 2.6 modifications
 * Revision 1.2  1992/12/18  19:10:48  glenn
 * Transfer from STE
 *
 * Revision 1.2.1.2  92/10/09  20:01:22  boris
 * Fix comment
 * 


   END-LOG---------------------------------------------

*/
