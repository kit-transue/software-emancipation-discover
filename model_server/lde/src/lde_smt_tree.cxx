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
#if 0
// lde_smt_tree.C
//
// synopsis : Ldr tree builder for call trees.
//
// description: Builds a tree of ldr nodes starting from given
//              root. If up_tree_flag = 0 (default) goes down level
//              by level until reaches max depth (default = 3). 
//              For the nodes on each level gets directly called
//              functions and builds nodes for them.
//              Does the same in the up directions for functions
//              that call each node if up_tree_flag = 1.
//              Sets decoration if the node is unexploded ( at max
//              depth , for exmaple, or recursive calls).
//
//                         
//                         +-----------+        
//                         | ldr_root  |
//                         +-----------+
//                               |
//                               |
//              -------------------------------------- ....
//              |                |                   |
//       +-----------+     +-----------+       +-----------+        
//       | ldr_child1|     | ldr_child2|       | ldr_child3|
//       +-----------+     +-----------+       +-----------+ 
//            ....
//                        
//

#include "genError.h"
#include "ldrTypes.h"
#include "ldrNode.h"
#include "ddict.h"


#define  CALL_TREE_MAX_LEVEL 3

int MAX_TREE_LEVEL = CALL_TREE_MAX_LEVEL;


extern ldrChartNodePtr build_one_node(appTreePtr, int);

extern objArr * dd_get_func_called(appTreePtr xref_fdecl);  
extern objArr * dd_get_called_by(appTreePtr xref_fdecl);  

extern void   build_one_level( objSet &curr_set, int level, 
                        objSet &built_set,  int up_tree_flag,
                        int max_depth);

// Top Level tree builder. 


ldrChartNodePtr lde_smt_tree (appTreePtr xref_root, int level,
                              int max_depth, int up_tree_flag)
{

   ldrChartNodePtr      ldr_root;
   ldrFuncCallNodePtr   ldr_first;
   objSet               curr_set;      // set of current ldr nodes
                                       // at one level
   objSet               built_set;     // set of processed xref's
   int                  curr_level;
   objArr               *func_call_arr;   // functions calls

//------------------ Executable Code ----------------------------

   Initialize(lde_smt_tree);

   ldr_root  = build_one_node(xref_root, level);
   ldr_first = checked_cast(ldrFuncCallNode,ldr_root->get_first());

   func_call_arr  = dd_get_called_by(xref_root);
   if ( func_call_arr && func_call_arr->size() )
             ldr_first->top_end = 1;      // decorate if no up tree 

   curr_set.insert(ldr_root);

   curr_level = level;
   while (curr_level < max_depth - 1){
       curr_level ++;
       build_one_level(curr_set, curr_level, built_set, up_tree_flag,
                       max_depth);
       if ( curr_set.empty() )
             break;
   }

   ReturnValue(ldr_root);
}





void   build_one_level(objSet &curr_set,  int level, 
                       objSet &built_set, int up_tree_flag,
                       int max_depth)
{
   ldrFuncCallNodePtr   ldr_first;
   ldrChartNodePtr      ldr_child;
   ldrNodePtr           ldr_prev;
   ldrChartNodePtr      ldr_root;
   objArr               *func_call_arr,   // functions called by or 
                        *tmp_arr;         // calling the node
   ldrConnectionNodePtr tree_connect;
   ldrConnectorType     conn_type;
   appTreePtr           xref_curr;
   appTreePtr           xref_root;
   objSet               level_set;
   Obj                  *el;
   Obj                  *el1;


//------------------ Executable Code ---------------------------

   Initialize(build_one_level);

// go thru all ldr_roots of one level built at the previous step

   ForEach(el, curr_set){

      func_call_arr = NULL;

//    check if xref_root has been processed to stop recursion

      ldr_root = (ldrChartNodePtr )el;
      xref_root = checked_cast(appTree, ldrtree_get_apptree(
                                             ldr_root));
//    get all subroutines called by this node

      if ( up_tree_flag ){
          func_call_arr  = dd_get_called_by(xref_root);
          conn_type = ldrcUP_TREE;
      }
      else{
          func_call_arr  = dd_get_func_called(xref_root);
          conn_type = ldrcTREE;
      }

      if ( !built_set.includes(xref_root) ){
          built_set.insert(xref_root);
          if ( func_call_arr != NULL && func_call_arr->size()){

//             create connector from root to children ( or to parents )

               tree_connect = db_new(ldrConnectionNode, (conn_type));
               tree_connect->set_graphlevel(level);
               tree_connect->add_input(ldr_root->get_exit_symbol());

               ldr_prev = checked_cast(ldrNode, ldr_root->get_first());
               ForEach(el1, *func_call_arr){

//                for each call build a node and set connections

                  xref_curr = checked_cast(appTree, el1);
                  ldr_child  = build_one_node(xref_curr, level);

                  if ( up_tree_flag )             // set tree direction
                       ( checked_cast(ldrFuncCallNode, 
                          ldr_child->get_first() ))->upper_tree = 1;

                  level_set.insert(ldr_child);
                  ldr_prev->put_after(ldr_child);
                  tree_connect->add_output(ldr_child->get_entry_symbol());
                  ldr_prev = ldr_child;

//                set top/bottom end flag

                  if ( level == max_depth - 1){
                     ldr_first = checked_cast(ldrFuncCallNode,ldr_child->get_first());
                     appTreePtr tmp_xref =
                       checked_cast(appTree, ldrtree_get_apptree(
                                                   ldr_child));
                     if ( up_tree_flag ){
                          if ( (tmp_arr = dd_get_called_by(tmp_xref)) != NULL &&
                                tmp_arr->size() )
                               ldr_first->top_end = 1;
                     }
                     else
                          if ( (tmp_arr = dd_get_func_called(tmp_xref)) != NULL &&
                                tmp_arr->size() )
                               ldr_first->bottom_end = 1;
                  }
               }
               ldr_prev->put_after(tree_connect);
          }
      }
//    already processed at the upper level - recursive call;
//    stop build tree and set collapsed flag if more children exist

      else if (func_call_arr != NULL && func_call_arr->size()){
          ldr_first = checked_cast(ldrFuncCallNode,ldr_root->get_first());

          if (up_tree_flag)
              ldr_first->top_end = 1;
          else
              ldr_first->bottom_end = 1;
      }
      if (func_call_arr != NULL)
               delete func_call_arr;
   }
   curr_set = level_set;
   Return
}


//------------------ build one node ------------


// ------------------  SIMPLE NODE ------------------------------


ldrChartNodePtr build_one_node(appTreePtr app_node, int level)
{
   ldrChartNodePtr  new_node;

//------  Executable code ----------------------

   Initialize(build_one_node);
   
   new_node   = db_new(ldrChartNode, (ldrSOMETHING));
   if (app_node != NULL)   
       new_node->set_appTree(app_node);    

   new_node->set_graphlevel(level);
   new_node->set_role(ldrFDECL);
   ldrFuncCallNodePtr f_symbol = db_new(ldrFuncCallNode,
                                  (level ? ldrFUNC_CALL : ldrFUNC_CALL_ROOT,
				  app_node));
   f_symbol->set_graphlevel(level);
   f_symbol->set_role(ldrFDECL);
   new_node->set_exit_symbol(f_symbol);
   new_node->set_entry_symbol(f_symbol);
   new_node->put_first(f_symbol);

   ReturnValue(new_node);
}

/*
   START-LOG-------------------------------------------

   $Log: lde_smt_tree.cxx  $
   Revision 1.3 1996/02/23 15:03:32EST mg 
   Bug track: NA
   removed obsolete xrefEntry
Revision 1.2.1.5  1994/08/02  17:07:15  kws
Bug track: 7665
Make root of call tree graphically identifiable

Revision 1.2.1.4  1992/11/21  22:16:34  builder
typesafe casts.

Revision 1.2.1.3  1992/10/13  17:37:11  sergey
Fixed bug # 1582 (false decoration).

Revision 1.2.1.2  92/10/09  19:19:09  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
#endif
