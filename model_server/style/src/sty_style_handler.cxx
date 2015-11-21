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
// ste_style_handler.C
//------------------------------------------
#include <objOper.h>
#include <steStyle.h>
#include <ste_get.h>
#include <ste_send_message.h>
#include <representation.h>
#include <steDisplayNode.h>
#include <steDisplayHeader.h>
#include <steTextNode.h>
#include <steView.h>
#include <ste_array_handler.h>
#include <steAppStyle.h>
#include <ste_send_message.h>
#include <steSlot.h>
#include <ste.h>
#include <ste_default_tables.h>
#include <steFontHandler.h>
#include <steEpochStyle.h>
#include <steStyleHeader.h>
#include <steHeader.h>
#include <smt.h>
#include <ste_table_handler.h>
#include <ste_smt_interface.h>
#include <stePostScript.h>

init_relation(default_style_of_ste,1,relationMode::D,ste_of_default_style,1,NULL);
init_relation(hilite_style_of_ste,1,relationMode::D,ste_of_hilite_style,1,NULL);
init_relation(default_style_of_view,1,relationMode::D,view_of_default_style,1,NULL);
init_relation(hilite_style_of_view,1,relationMode::D,view_of_hilite_style,1,NULL);

void ste_propagate_style_apply (appTreePtr node) 
/*
 * Updates style/category on all views whre the specified node is visible 
 *
 */
{
    Initialize(ste_propagate_style_apply);

    // Check not deleted node
    if (!node || (node->get_id() <= 0)) 
	return;    

    Obj *obj_dnd;
    steViewPtr cur_view;
    steDisplayNodePtr dnd;
    steDisplayHeaderPtr dnd_head;
    Obj *dnd_set;

    if (is_appTree(node))
        dnd_set = get_relation(ldrtree_of_apptree,node);
    else
        dnd_set = (Obj *)node;
        
    ForEach (obj_dnd, *dnd_set) {
	dnd = checked_cast(steDisplayNode,obj_dnd);
	if (!dnd || (dnd->get_id() <= 0))
	    continue;
	
	dnd_head = checked_cast(steDisplayHeader,dnd->get_header());
	if (!dnd_head || (dnd_head->get_id() <= 0))
	    continue;
	
	if ( !( (dnd_head->get_type() == Rep_TextText) ||
		(dnd_head->get_type() == Rep_SmtText) ) )
	    continue;
	
	Obj *ob = get_relation(view_of_ldr,dnd_head);
	Obj *cc_ob;
	ForEach(cc_ob,*ob){
	    cur_view = checked_cast(steView,cc_ob);
	    cur_view->ste_start_transaction();
	    cur_view->build_styles (dnd);
	    cur_view->ste_end_transaction();
	}
    }
}


/************************************
* get_default_app_style
************************************/
steAppStylePtr get_default_app_style( int app_type ) {
    Initialize(get_default_app_style);

    steAppStylePtr answer = checked_cast(steAppStyle,get_app_table(app_type, Ste_Style));
    return answer;
}

/************************
* get_create_app_style( appPtr )
*************************/
steAppStylePtr get_create_app_style( appPtr appl) {
    int app_type = appl->get_type();
    steAppStylePtr cur;
 Initialize(get_create_app_style);
    steAppStylePtr answer = checked_cast(steAppStyle,get_relation(style_of_app,appl));
    if ( answer == NULL ) {
        cur = get_default_app_style(app_type);
        answer = db_new(steAppStyle,(Ste_Local,cur));
        answer->set_app(appl);
    }   
 ReturnValue(answer);
}

/************************************
* steSlotPtr get_node_style();
************************************/
steSlotPtr get_node_style(commonTreePtr nd) {
    Initialize(get_node_style);

     commonTreePtr w_n;
     if ( is_steDisplayNode( nd ) )
       w_n = checked_cast(commonTree,get_relation_apptree_of_ldrtree( checked_cast(ldrTree,nd )));

     if (!w_n) return NULL;
     return checked_cast(steSlot,get_relation(style_of_node, w_n));
}
//
//
//
steSlotPtr get_node_character_style( commonTreePtr node, steViewPtr c_view ) {
 Initialize(get_node_character_style);
    commonTreePtr w_n;
    steSlotPtr answer = NULL;

    if (!c_view) c_view = get_current_view();

    IF( !node ) ReturnValue(answer);

    steSlotPtr cat = checked_cast(steSlot,get_relation(category_of_node,node));
    if ( !cat ) {
      if ( (w_n = checked_cast(commonTree,get_relation(apptree_of_ldrtree,node))) )
          cat = checked_cast(steSlot,get_relation(category_of_node,w_n));
    }
    int flag_on_cat = 0;    // so far !
    if (cat && flag_on_cat){ 
       steCategoryPtr cc = checked_cast(steCategory,cat->get_slot_obj());
       answer = checked_cast(steSlot,cc->get_category_styleslot());
    }else
       answer = get_node_style(node);

 ReturnValue(answer);
}

/************************************
* steAppStylePtr get_node_style_table();
************************************/
steAppStylePtr get_node_style_table( commonTreePtr node ) {
 Initialize(get_node_style_table);
    steAppStylePtr answer = NULL;
    HierarchicalPtr head;
    if ( is_steDisplayNode(node) )
       head = checked_cast(Hierarchical, checked_cast(steDisplayNode,node)->find_first_header());
    else
       head = node->get_header();   
 
    answer = checked_cast(steAppStyle,get_relation(style_of_app,head));
    if (!answer) {
        appPtr appl = checked_cast(app,get_relation(app_of_ldr,head));
        if (appl) answer = checked_cast(steAppStyle,get_create_app_style(appl));
    }
 ReturnValue(answer);
}

/****************
* get_font_handler()
****************/
steFontHandlerPtr get_font_handler(steEpochStylePtr st) {
    Initialize(get_font_handler);

   steStyleHeaderPtr head = checked_cast(steStyleHeader,st->get_header());
   steFontHandlerPtr f_hand = 
      checked_cast(steFontHandler,get_relation(fonthandler_of_styleheader,head));
   return f_hand;
}


/*******************
* get_style_by_name()
********************/
steSlotPtr get_style_by_name(const char *nm){
 Initialize(get_style_by_name);
   Assert(ste_main);
   steSlotPtr sl = NULL;
   steAppStylePtr st_tbl;
   ldrPtr ldr_hd;
   appPtr cur_app;
   steViewPtr cur_view = NULL;

      if ( !cur_view )
          cur_app = get_current_app();
      else{
       ldr_hd = cur_view->get_ldrHeader();
       cur_app = ldr_hd->get_appHeader();
      }
      if ( cur_app )
           st_tbl = checked_cast(steAppStyle,get_relation(style_of_app,cur_app));
      else
           st_tbl = checked_cast(steAppStyle,get_relation(global_style_of_ste,ste_main));

      if (!st_tbl) ReturnValue (NULL);
      sl = st_tbl->get_slot( nm );
    
 ReturnValue(sl);
}

/**********************
*  get_style_by_name
**********************/
steSlotPtr get_style_by_name (const char *st, appPtr appl) {
    Initialize(get_style_by_name);

   steSlotPtr answer = NULL;
   steAppStylePtr tbl_s = NULL;
 
   if (appl)
      tbl_s = get_create_app_style (appl);

   answer = (tbl_s) ? tbl_s->get_slot(st) : NULL;
 return answer;
}
 
 
/**********************
*  get_style_by_name
**********************/
steSlotPtr get_style_by_name (const char *st, appTreePtr nd) {
 Initialize(get_style_by_name);
   appPtr appl ;
   if (nd)
       appl = checked_cast(app,nd->get_header());
   else
       appl = get_current_app();
   steSlotPtr answer = get_style_by_name (st, appl);
   return answer;
}
 
void ste_interface_propagate_style_change( steSlotPtr );



/**************
* get_epochstyle_view()
**************/
steViewPtr get_epochstyle_view( steEpochStylePtr p ){
 Initialize(get_epochstyle_view);
    steViewPtr answer = NULL;

    if ( !p ) ReturnValue(answer);

    steStyleHeaderPtr hd = checked_cast(steStyleHeader,p->get_header());
    answer = checked_cast(steView,get_relation(view_of_style_header,hd));
 ReturnValue(answer);
}


/*******
* ste_is_splice_node_style();
*********/
int ste_is_splice_node_style(commonTreePtr nd){
 Initialize(ste_is_splice_node_style);
// 0 - no; 1 - yes;

   int tp = nd->get_node_type();
   if ( tp != steSTR ) 
      ReturnValue(0);

   if ( !is_steDisplayNode(nd) )
      ReturnValue(1);

   appTreePtr ap_nd = checked_cast(ldrTree,nd)->get_appTree();
   if ( !ap_nd )
      ReturnValue(1);  // I do not like steSTR without app nodes (steFOOT);

   commonTreePtr frst = checked_cast(commonTree,nd->get_first());
   if ( frst->get_node_type() == steSTR )
        ReturnValue(1);

   if ( !is_smtTree(ap_nd) )   
      ReturnValue(0);

   int smt_tp = checked_cast(smtTree,ap_nd)->type;
   if ( smt_tp == SMT_cbody )
       ReturnValue(1);

   if (  smt_tp == SMT_block ){
       smtTreePtr prnt = checked_cast(smtTree,ap_nd->get_parent());         
       if ( prnt && prnt->type != SMT_block )
          ReturnValue(1);
   }
   ReturnValue(0);
}

/********
* ste_get_smt_title_type()
********/
int ste_get_smt_title_type(commonTreePtr nd){
 Initialize(ste_get_smt_title_type);
// returns block brace type : 0 - no; 1 - start; 2- end;

   int tp = nd->get_node_type();
   commonTreePtr frst = checked_cast(commonTree,nd->get_first_leaf());
   if ( !frst || tp != steHEAD || !is_steDisplayNode(nd) ) 
      ReturnValue(0);

   appTreePtr ap_nd = checked_cast(ldrTree,frst)->get_appTree();
   if ( !ap_nd || !is_smtTree(ap_nd) )   
      ReturnValue(0);

   if ( checked_cast(smtTree,ap_nd)->type == SMT_token  && 
        checked_cast(smtTree,ap_nd)->extype == SMTT_begin )
          ReturnValue(1);

   if ( checked_cast(smtTree,ap_nd)->type == SMT_token  && 
        checked_cast(smtTree,ap_nd)->extype == SMTT_end )
          ReturnValue(2);

   ReturnValue(0);
}

/********
* ste_get_smt_clause_style()
*********/
// returns 1 if nd corresponds to else/else if clause;
//
int ste_get_smt_clause_style(commonTreePtr nd){
 Initialize(ste_get_smt_clause_style);
   int tp = nd->get_node_type();
   if ( tp != steHEAD || !is_steDisplayNode(nd) ) 
      ReturnValue(0);

   appTreePtr ap_nd = ste_get_target_app_node( nd );
   if ( !ap_nd || !is_smtTree(ap_nd) )
      ReturnValue(0);

   int prnt_tp =  checked_cast(smtTree,ap_nd)->type; 
   if ( prnt_tp == SMT_else_clause || prnt_tp == SMT_else_if_clause )
       ReturnValue(1);

   ReturnValue(0);
}

/********
* ste_is_base_style_type ()
********/
int ste_is_base_style_type ( ste_style_type tp ){
 Initialize(ste_is_base_style_type);
      if( tp != TITLE_LEVEL_STYLE     &&
          tp != PARAGRAPH_LEVEL_STYLE &&
          tp != TABLE_NAME_STYLE      &&
          tp != LEFT_CORNER_STYLE     &&
          tp != ZERO_COLUMN_STYLE     &&
          tp != ZERO_ROW_STYLE        &&
          tp != DEFAULT_ITEM_STYLE    &&
          tp != FULL_COMMENT_STYLE    &&
          tp != INLINE_COMMENT_STYLE
        )
      ReturnValue(0);
   else
      ReturnValue(1);

}

/********
* ste_get_smt_comment_type()
********/
int ste_get_smt_comment_type(commonTreePtr nd){
    Initialize(ste_get_smt_comment_type);

   int tp = nd->get_node_type();
   commonTreePtr frst = checked_cast(commonTree,nd->get_first());
   if ( !frst || tp != stePAR || !is_steDisplayNode(nd) ) 
      return 0;

   appTreePtr ap_nd = checked_cast(ldrTree,frst)->get_appTree();
   if ( !ap_nd )
      return 0;

   if ( !is_smtTree(ap_nd) )   
      return 0;

   if ( checked_cast(smtTree,ap_nd)->type == SMT_token  && 
        checked_cast(smtTree,ap_nd)->extype == SMTT_commentl )
          return FULL_COMMENT_STYLE;

   if ( checked_cast(smtTree,ap_nd)->type == SMT_token  && 
        checked_cast(smtTree,ap_nd)->extype == SMTT_comment )
          return INLINE_COMMENT_STYLE;

   return 0;
}


/*******
* ste_get_node_style_type()
*******/
int ste_get_node_style_type ( commonTreePtr root ){
       int root_type = root->get_node_type();
       int st_type = ste_get_table_item_type( root );
       if (st_type == 0){
          st_type = ste_get_smt_comment_type( root );
          if ( st_type == 0 )
              st_type = ( root_type == stePAR ) ? PARAGRAPH_LEVEL_STYLE :
                                                  TITLE_LEVEL_STYLE;
       }
 return st_type;
}
