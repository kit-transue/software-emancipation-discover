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
////////////////////////   FILE ste_category_handler.C   ////////////////////
//
// -- Contains functions for handling categfories.
//
//         boolean root_category_test(commonTreePtr nd, steCategoryPtr categ );
//
#define  _cLibraryFunctions_h
#include "objOper.h"
#include "Object.h"
#include "ste_actions.h"
#include "ste_smt_interface.h"
#include "steBufferHeap.h"
#include "steTextNode.h"
#include "steRefNode.h"
#include "steDocument.h"
#include "genError.h"
#include "steCategory.h"
#include "steScreen.h"
#include "objTree.h"
#include "steDisplayNode.h"
#include "steView.h"
#include "ste.h"
#include "steHeader.h"
#include "ste_array_handler.h"
#include "ste_common_apply.h"
#include "ste_build_ref.h"
#include "ste_get.h"
#include "ste_common_apply.h"
#include "ste_category_interface.h"
#include "ste_category_handler.h"
#include "ste_cut.h"
#include "ste_mouse.h"
#include "ste_common_apply.h"
#include "oodt_interface.h"
#include "steAppCategory.h"
#include "ste_default_tables.h"
#include "steSlot.h"
#include "ste_format_handler.h"
#include "ste_style_handler.h"
#include "ste_interface.h"
#include "smt.h"
#include "smode_categories.h"
#include "ste_categ_enum.h"
#include "steStyle.h"
#include "driver.h"
#include "ste_smod.h"
 
#undef Message
 
/***************************
*  root_category_test
***************************/
boolean root_category_test( commonTreePtr nd, steSlotPtr categ ) {
 Initialize(root_category_test);
   commonTreePtr w_txt;
   Obj *cat_set;
 
   commonTreePtr root = (reference_get_root(checked_cast(appTree, nd)))
            ? reference_get_root(checked_cast(appTree, nd))
            :  nd;
 
   cat_set = get_relation(category_of_node, root);
   if(cat_set == 0) ReturnValue(false);
 
   if (cat_set->includes((Obj *)categ) )
             ReturnValue(true);
       else{
             w_txt = checked_cast(commonTree,root->get_parent());
             if ( w_txt )
                   ReturnValue(root_category_test(w_txt, categ));
             else
                   ReturnValue(false);
       }
}
 
 
//
// ste_get_item_header
//
steTextNodePtr ste_get_item_header( steTextNodePtr txt ) {
 Initialize(ste_get_item_header);
     steTextNodePtr w1, w2;
     steTextNodePtr w_txt = NULL;
     for(w1 = txt; w1; w1 = checked_cast(steTextNode,w1->get_parent()))
          if( w1->get_node_type() == steSTR ) {
                w2 = checked_cast(steTextNode,w1->get_first());
                if (w2->get_node_type() == steHEAD){
                    w_txt = w1;
                    break;
                }
          }
 ReturnValue(w_txt);
}
//
// remove_category_item
//
void ste_remove_category_item (steTextNodePtr w_txt)
{
    Initialize(ste_remove_category_item);
    w_txt = ste_get_item_header( w_txt );
    if ( w_txt )
	ste_remove_text_node( w_txt );
    else 
    {
	steHeader(Message)
	    << " - wrong item structure"
	    << steEnd;
    }
}

//
//
//
void remove_category_from_node (appTreePtr ap_tree, steSlotPtr cat) 
{
 
    appTreePtr w_app;
    Obj *w_obj;
    Obj *txt_set;
    steTextNodePtr txt;
    Initialize(remove_category_from_node);
 
    if (root_category_test (checked_cast(commonTree,ap_tree), cat))
    {
	//---------the uppermost parent with this category;
	for ( w_app = ap_tree; w_app;
	      w_app = checked_cast(appTree,ap_tree->get_parent() ))
	{
	    if ( root_category_test(checked_cast(commonTree,w_app), cat ))
		ap_tree = w_app;
	    else
		break;
	}

	rem_relation(category_of_node, ap_tree, cat);
	obj_assign(ap_tree->get_header(), ap_tree, cat, 0);
 
	if (!is_steTextNode(ap_tree))
	{
	    ste_propagate_style_apply(checked_cast(appTree,ap_tree));
	    return;
	}
	txt = checked_cast(steTextNode,ap_tree);
	if (is_steRefNode(txt) ){
	    txt = checked_cast(steTextNode,get_relation(reference_of_startnode, txt));
	    remove_category_from_node( txt, cat );
	}else{
	    txt->remove_category( cat );
	    ste_propagate_style_apply(checked_cast(appTree,txt));
	    txt_set = get_relation(reference_of_startnode, txt);
	    if ( txt_set ) {
		if (txt_set->collectionp()) {
		    ForEach(w_obj, *txt_set) {
			steTextNodePtr w_txt = checked_cast(steTextNode,w_obj);
			ste_remove_category_item( w_txt );
		    }
		}else
		    ste_remove_category_item( checked_cast(steTextNode,txt_set ));
	    }
	}
    }
    Return
    }
 
 
 
/************************************
* get_default_app_system_category
************************************/
steAppCategoryPtr get_default_app_system_category( int app_type ) {
    Initialize(get_default_app_system_category);

    steAppCategoryPtr answer =
           checked_cast(steAppCategory,get_app_table(app_type,System_Category));
    return answer;
}
 
/************************************
* get_default_app_user_category
************************************/
steAppCategoryPtr get_default_app_user_category( int app_type ) {
    Initialize(get_default_app_user_category);

    steAppCategoryPtr answer =
          checked_cast(steAppCategory,get_app_table(app_type,User_Category));
    return answer;
}
 
/************************
* get_create_local_system_category( appPtr )
*************************/
steAppCategoryPtr get_create_local_system_category( appPtr appl) {
    Initialize(get_create_local_system_category);

    int app_type = appl->get_type();
    steAppCategoryPtr cur;
    steAppCategoryPtr answer = checked_cast(steAppCategory,
            get_relation(system_category_of_app,appl));
    if ( answer == NULL ) {
        cur = get_default_app_system_category(app_type);
        answer = db_new(steAppCategory,(Ste_Local,checked_cast(steSlotTable,cur)));
        put_relation(system_category_of_app,appl,answer);
    }
 return answer;
}
 
/************************
* get_create_local_user_category( appPtr )
*************************/
steAppCategoryPtr get_create_local_user_category( appPtr appl) {
    Initialize(get_create_local_user_category);

    int app_type = appl->get_type();
    steAppCategoryPtr cur;
    steAppCategoryPtr answer = checked_cast(steAppCategory,
            get_relation(user_category_of_app,appl));
    if ( answer == NULL ) {
        cur = get_default_app_user_category(app_type);
        answer = db_new(steAppCategory,(Ste_Local,checked_cast(steSlotTable,cur)));
        put_relation(user_category_of_app,appl,answer);
    }
 return answer;
}
 
/**********************
*  get_category_by_name
**********************/
steSlotPtr get_category_by_name( const char *st, appPtr appl ) {
    Initialize(get_category_by_name);

   steSlotPtr answer = NULL;
   steAppCategoryPtr tbl_s = NULL;
   steAppCategoryPtr tbl_u = NULL;
 
   if (appl) {
      tbl_s = get_create_local_system_category(appl);
      tbl_u = get_create_local_user_category(appl);
   }else{
      tbl_s = checked_cast(steAppCategory,ste_main->get_default(System_Category));
      tbl_u = checked_cast(steAppCategory,ste_main->get_default(User_Category));
   }
   answer = tbl_u->get_slot(st);
   if (answer == NULL) answer = tbl_s->get_slot(st);
 
 return answer;
}
 
 
/**********************
*  get_category_by_name
**********************/
steSlotPtr get_category_by_name( const char *st, viewPtr cur_view ) {
 Initialize(get_category_by_name);
   ldrPtr ldr_hd;
   appPtr appl;
 
   if ( !cur_view )
       appl = get_current_app();
   else{
       ldr_hd = cur_view->get_ldrHeader();
       appl = ldr_hd ? ldr_hd->get_appHeader() : NULL;
   }
 
   steSlotPtr answer = get_category_by_name( st, appl );
 ReturnValue(answer);
}
 
 
/**********************
*  get_category_by_name
**********************/
steSlotPtr get_category_by_name( const char *st, appTreePtr nd ) {
 Initialize(get_category_by_name);
   appPtr appl ;
   if ( nd )
       appl = checked_cast(app,nd->get_header());
   else
       appl = get_current_app();
   steSlotPtr answer = get_category_by_name( st, appl );
 ReturnValue(answer);
}
 
 
/**********************
*  get_category
**********************/
steSlotPtr get_category( int st, appPtr appl ) {
    Initialize(get_category);

   steSlotPtr answer = NULL;
   steAppCategoryPtr tbl_s = NULL;
   steAppCategoryPtr tbl_u = NULL;
 
   if (appl) {
      tbl_s = get_create_local_system_category(appl);
      tbl_u = get_create_local_user_category(appl);
   }else{
      tbl_s = checked_cast(steAppCategory,ste_main->get_default(System_Category));
      tbl_u = checked_cast(steAppCategory,ste_main->get_default(User_Category));
   }
   answer = tbl_u->get_slot(st);
   if (answer == NULL) answer = tbl_s->get_slot(st);
 
 return answer;
}
 
 
 
/**********************
*  get_category
**********************/
steSlotPtr get_category( int st, appTreePtr nd ) {
 Initialize(get_category);
   appPtr appl ;
   if ( nd )
       appl = checked_cast(app,nd->get_header());
   else
       appl = get_current_app();
   steSlotPtr answer = get_category( st, appl );
 ReturnValue(answer);
}
 
 
//
//  add_category
//
 
void ste_add_category (appTreePtr node, steSlotPtr ctg) 
{
    Initialize(ste_add_category);
    if (!root_category_test (checked_cast(commonTree,node), ctg))
    {
	put_relation(category_of_node, node, ctg);
	obj_assign(node->get_header(), node, ctg, 1);
    }
 
    if (is_smtTree(node))
	smod_rebuild_node (checked_cast(smtTree,node), SmodChange);

    Obj *ldr_set = apptree_get_ldrtrees(node);
    if (ldr_set == NULL)
	return;
 
    Obj *ldr_el;
    ldrTreePtr dnd;
    steDisplayHeaderPtr dnd_head;
    steViewPtr cur_view;
 
    ForEach(ldr_el,*ldr_set)
    {
	dnd = checked_cast(ldrTree,ldr_el);
	dnd_head = checked_cast(steDisplayHeader,dnd->get_header());
	if ( dnd_head == NULL ) continue;
	if ( !( (dnd_head->get_type() == Rep_TextText) ||
		(dnd_head->get_type() == Rep_SmtText) ) )
	    continue;
	Obj *ob = ldr_get_views(dnd_head);
	if (!ob) continue;
	Obj *cc_ob;
	ForEach(cc_ob,*ob)
	{
	    cur_view = checked_cast(steView,cc_ob);
	    if (cur_view && cur_view->get_type() == Rep_TextText)
		cur_view->assign_style( checked_cast(steDisplayNode,dnd), ctg );
	}
    }
}
 
/*******************
* get_node_category()
*******************/
Obj * get_node_category( commonTreePtr nd ) {
    Initialize(get_node_category);

     commonTreePtr my_nd = NULL;
     if (is_ldrTree(nd)) my_nd = (checked_cast(ldrTree,nd))->get_appTree();
 
     Obj *ob =  NULL;
     if( my_nd )
         ob = get_relation(category_of_node,my_nd);
 return ob;
}
 
static ste_categ_no storage_class_categ[] = {
   ste_categ_File_Static,               // SMODE_FILE_STATIC,
   ste_categ_Local_Static,              // SMODE_LOCAL_STATIC,
   ste_categ_Local_Auto,                // SMODE_LOCAL_AUTO,
   ste_categ_Global,                    // SMODE_GLOBAL,
   ste_categ_Member,                    // SMODE_MEMBER,
   ste_categ_Parameter,                 // SMODE_PARAMETER,
   ste_categ_Keyword,                   // SMODE_KEYWORD,
   ste_categ_Function,                  // SMODE_FUNCTION
};
 
static ste_categ_no datatype_categ[] = {
   ste_categ_char,                      // SMODE_CHAR,
   ste_categ_short,                     // SMODE_SHORT,
   ste_categ_int,                       // SMODE_INT,
   ste_categ_long,                      // SMODE_LONG,
   ste_categ_charptr,                   // SMODE_STRING,
   ste_categ_float,                     // SMODE_FLOAT,
   ste_categ_double,                    // SMODE_DOUBLE,
   ste_categ_unknown,                   // SMODE_USER_TYPE,
   ste_categ_pointer,                   // SMODE_POINTER,
   ste_categ_reference,                 // SMODE_REFERENCE,
   ste_categ_array,                     // SMODE_ARRAY,
   ste_categ_unknown                    // SMODE_UNKNOWN_TYPE
};
 
extern void smt_get_smode_categories(smtTreePtr, smode_storage_class&,
                                     smode_datatype&, char*, int);
 
/********
* ste_get_node_default_categories()
*********/
void ste_get_node_default_categories(commonTreePtr root, bool dec_source,
                                     objArr& categories) {
    Initialize(ste_get_node_default_categories);

   int raw_f = 0;
 
   if ( is_steDisplayNode(root) ){
      raw_f = checked_cast(steDisplayNode,root)->raw_flag;
      root = checked_cast(commonTree,ste_get_target_app_node( root ));;
   }
   if ( !root || !is_smtTree(root))
         return;
    
   smtTreePtr smt_nd = checked_cast(smtTree,root);
   if ( raw_f == 1       ||
        smt_nd->ndm == 1 ||
        ( !root->get_first() && smt_nd->arn == 1))   {
#if 0
      categories.insert_last(get_category(ste_categ_Suspended_Parse,
                                          checked_cast(appTree,root)));
#endif
      return;
   }

   if ( dec_source == 0 )
      return;

   int tp = smt_nd->type;
   if ( tp != SMT_token )
      return;
    
   int ex_tp = smt_nd->extype;
   if ( ex_tp == SMTT_ident ) {
      const int MAX_LEN = 256;
      char type_str[MAX_LEN];
      smode_storage_class sc;
      smode_datatype dt;
      smt_get_smode_categories(smt_nd, sc, dt, type_str, MAX_LEN);
      appPtr appl = checked_cast(app,root->get_header());
      categories.insert_last(get_category(storage_class_categ[sc], appl));
      if (dt == SMODE_USER_TYPE || dt == SMODE_POINTER ||
          dt == SMODE_REFERENCE || dt == SMODE_ARRAY) {
         steSlotPtr sl = get_category_by_name(type_str, appl);
         if (sl)
            categories.insert_last(sl);
         else categories.insert_last(get_category(datatype_categ[dt], appl));
      }
      else categories.insert_last(get_category(datatype_categ[dt], appl));
   }
   else if ( ex_tp == SMTT_kwd )
      categories.insert_last(get_category(ste_categ_Keyword,
                                          checked_cast(appTree,root)));
   else if ( ex_tp == SMTT_grouping )
      categories.insert_last(get_category(ste_categ_Grouping,
                                          checked_cast(appTree,root)));
   return;
}
