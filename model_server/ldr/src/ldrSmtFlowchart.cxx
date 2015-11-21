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
// ldrSmtFlowchart.h.C
//
// ldrSmtFlowchart  is an ldr  header for flowchart
// diagrams of Abstract Syntax Tree (Smt).


#include "genError.h"
#include "objOperate.h"
#include "ldrError.h"
#include "ldrNode.h"
#include "ldrSelection.h"
#include "ldrSmtFlowchart.h"
#include "lde-flowchart.h"
#include "OperPoint.h"
#include <ddict.h>
#include <xrefSymbol.h>

init_relational(ldrSmtFlowchart,ldrHeader);

extern void flow_build_selection(const ldrSelection& ldr_inp,
				   OperPoint& app_point);
ddElement* smt_get_dd(smtTree*);
bool smtTree_get_symbol(smtTree*node, symbolPtr& sym)
{
   bool retval = false;
   if(node->get_parent()){
     ddElement* dd = smt_get_dd(node);
     if(dd){
       sym = dd->get_xrefSymbol();
       retval = true;
     }
   } else {
     smtHeader* header = (smtHeader*) node->get_header();
     if(header){
       symbolPtr hsym = header;
       sym = hsym.get_xrefSymbol();
       retval = true;
     }
   }
#if 0
   if(retval)
       cout << "smtTree_get_symbol " << sym.get_name() << endl;
   else
     cerr << "smtTree_get_symbol " << sym.get_name() << 
       " does not have definition" << endl;
#endif
   return retval;
}

smtTree* symbol_get_smtTree(symbolPtr& sym)
{
 Initialize(symbol_get_smtTree);
  smtTree* node = NULL;
  switch(sym.get_kind()){
  case DD_MODULE:
    {
      app*header = sym->get_def_file();
      if(header)
	node = checked_cast(smtTree, header->get_root()); 
      break;
    }  
  default:
    {
      ddElement* dd = sym.get_def_dd();
      if(dd){
	node =  (smtTree*) get_relation(def_smt_of_dd, dd);
      }
      break;
    }
  }
//  cout << "symbol_get_smtTree " << sym.get_name() << ' ' << node << endl;
  return node;
}

ldrSmtFlowchart::ldrSmtFlowchart(smtTreePtr tr)
{
   Initialize(ldrSmtFlowchart::ldrSmtFlowchart);
   Assert(smtTree_get_symbol(tr, symbol));
   put_root(checked_cast(objTree, lde_build_smt_flowchart(tr, 0)));
}

ldrSmtFlowchart::ldrSmtFlowchart(smtHeaderPtr app_header,
				 smtTreePtr sub_tree)
{
   Initialize(ldrSmtFlowchart::ldrSmtFlowchart);

   set_type(Rep_FlowChart);
   if (sub_tree == NULL)
     sub_tree = checked_cast(smtTree, app_header->get_root());
   Assert(smtTree_get_symbol(sub_tree, symbol));
   put_relation(app_of_ldr, this, app_header);
   put_root(checked_cast(objTree, lde_build_smt_flowchart(sub_tree, 0)));
   return;
}

ldrSmtFlowchart::ldrSmtFlowchart(const ldrSmtFlowchart &ll) 
   { *this = ll;}


// Optimal implementation  of  insert_obj  is   too
// complicated for now --  just redo the whole ldr.
// Once ldr for  flowchart  is settled, this method
// should be rewritten to run faster.

void ldrSmtFlowchart::insert_obj(objInserter* old_inserter,
				 objInserter* new_inserter)
{
   Initialize(ldrSmtFlowchart::insert_obj);
   if ( old_inserter->src_obj  == NULL &&
        old_inserter->targ_obj == NULL &&  
        old_inserter->data     != NULL &&
        old_inserter->type     == SPLIT )
      new_inserter->type = NULLOP;
   else {
       // Convert REPLACE_REGION into REGENERATE
       if (old_inserter->type == REPLACE_REGION) {
	   new_inserter->type = REPLACE;
	   new_inserter->data = NULL;
	   new_inserter->src_obj = new_inserter->targ_obj = 
	       obj_tree_common_root (old_inserter->src_obj, old_inserter->targ_obj);
       }
       redo_insert(new_inserter);
   }
}

// Real removal is done by lde_detach_flowchart
// which knows more about the structure of  the ldr
// tree.

void
ldrSmtFlowchart::remove_obj(objRemover* old_remover,
			    objRemover* new_remover)
{
   ldrNodePtr src_ldr;
   Initialize(ldrSmtFlowchart::remove_obj);
   src_ldr = checked_cast(ldrNode,find_ldr(old_remover->src_obj));
   if (src_ldr != NULL) {
      lde_detach_flowchart((ldrChartNodePtr) src_ldr);
      src_ldr->remove_from_tree();
      new_remover->src_obj = src_ldr;
   }
}

// Instead   of  making local    tree  surgery, the
// following functions  just rederives   the  whole
// tree. It then  changes the inserter  to  that of
// replace of  the  whole tree  so  the  change  is
// propagated properly.

void ldrSmtFlowchart::redo_insert(objInserter* new_inserter)
{
   Initialize(ldrSmtFlowchart::redo_insert);

   ldrNode* old_root = checked_cast(ldrNode,get_root());
   smtTree* smt_root = symbol_get_smtTree(symbol);
   ldrNode* new_root = NULL;
   if (smt_root)
     {
       new_root = lde_build_smt_flowchart(smt_root, 0);
       put_root(new_root);
     }
   new_inserter->type = NULLOP;
   new_inserter->src_obj = new_root;
   new_inserter->targ_obj = old_root;
   
   if (new_root)
     obj_delete(old_root);
   else
     obj_delete(this);
}

static ldrSymbolNodePtr get_single_input(ldrConnectionNodePtr conn);

// When   a connector is selected,   it must have a
// single input, in which case it is interpreted as
// an insertion  AFTER the input with the exception
// of the case when the input has type ENTRY_SYMBOL
// -- then it means insertion of the FIRST child of
// the  parent  of the input.   All  non-conforming
// selections raise an error.

void ldrSmtFlowchart::build_selection(const ldrSelection& ldr_inp, OperPoint& app_point)
{
   flow_build_selection(ldr_inp, app_point);
}




// common call for Flowchart and Tree - should extracted from here

void flow_build_selection(const ldrSelection& ldr_inp,
				   OperPoint& app_point)
{
   ldrNodePtr   ldr_node = ldr_inp.ldr_symbol;
   ldrNodePtr   ldr_parent;
   ldrRole      role;
   appTreePtr   app_parent;
   appTreePtr   app_next;
   int          decl_flag;

// ------------ Executable Code ------------------

   Initialize(flow_build_selection);

// get app node attached to ldr node 

   app_point.node = ldr_node->get_appTree();
   app_point.type = AFTER;

// if destination is EXIT - insert after PARENT (block),
// LOOP, etc.

   if (ldr_node->get_role() == ldrEXIT){
       ldr_parent = checked_cast(ldrNode,get_relation(exit_from,ldr_node));
       app_point.node = ldr_parent->get_appTree();       
   }

// ENTRY node (of a block)
//     insert FIRST in smt block which exists unless
//     it is "ldr only" block appeared in no-block ifs, loops, etc.
//     skip declaration stsmts if any unless inside CLASS body

   else if (ldr_node->get_role() == ldrENTRY){
       app_point.type = FIRST;
       ldr_parent = checked_cast(ldrNode,get_relation(entry_into,ldr_node));
       app_parent = ldr_parent->get_appTree();      // block

//     ldr only block ?
       if ((checked_cast(smtTree,app_parent))->type != SMT_block)
           app_parent = checked_cast(smtTree, app_parent->get_parent()); // clause
                                                    // more iteration??
//     insertion before decls can be a problem in "C" case

       app_point.node = app_parent;
       app_next = checked_cast(appTree, app_parent->get_first());  // group ?
       app_next = checked_cast(appTree, app_next->get_next());
       if ( app_next != NULL &&
            (checked_cast(smtTree,app_next))->type == SMT_decl &&
            (checked_cast(smtTree,app_parent))->type != SMT_cbody ){

//         put AFTER first decl; more decls will be skipped below
           app_point.node = app_next;
           app_point.type = AFTER;         
       }
   }

// regular case

   else {
       ldr_parent = ldr_node;
       while (!(int) app_point.node){  
           ldr_parent = checked_cast(ldrNode,ldr_parent->get_parent());
           app_point.node = ldr_parent->get_appTree();       
       }

//     if LOOP's title, body, or footer selected - select
//     the entire LOOP; simillar logic is for IF selection
//
       role = ldr_parent->get_role();
       if ( role == ldrLOOP_TITLE ||
            role == ldrLOOP_BODY  ||
            role == ldrLOOP_FOOTER )
       {
           ldr_parent = checked_cast(ldrNode,ldr_parent->get_parent());
           app_point.node = ldr_parent->get_appTree();       
       }

       else if ( role == ldrIF_TEST ||
                 role == ldrELSE  )
       {
           ldr_parent = checked_cast(ldrNode,ldr_parent->get_parent());
           app_point.node = ldr_parent->get_appTree();       
       }

   }
// selected node always goes after selected destination
// if the destination is a simple construction (stmt, 
// decl, entry, etc.); if the distination is a declaration node
// insert after all following declarions unless inside CLASS body;
// if the destination is part of LOOP, IF, SWITCH, 
// the node goes after the entire construction (parent
// of selected destination).

//   app_parent = checked_cast(smtTree,app_point.node->get_parent());
   appTreePtr app_point_node = app_point.node;
   app_parent = checked_cast(smtTree,app_point_node->get_parent());
   if ( (checked_cast(smtTree,app_parent))->type != SMT_cbody ){
        decl_flag = 0;
        while ( (checked_cast(smtTree,app_point_node))->type == SMT_decl){
            decl_flag = 1;
            app_point_node = checked_cast(smtTree,app_point_node->get_next());
            if ( app_point_node == NULL )
                   Error(ERR_FAIL);
        }


//      now make a step back to insert after the last decl.

        if (decl_flag == 1)
           app_point.node = checked_cast(smtTree,app_point_node->get_prev());
	else
	   app_point.node = app_point_node;
   }
}









