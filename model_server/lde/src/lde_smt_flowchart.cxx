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
// lde_smt_flowchart.C
//
// synopsis : Creates ldr nodes for flowchart from smt objects.
//
//
// History: 12/19/91     S.Spivakovksy      Initial coding.
//                       some code borrowed from lde-flowchart.C

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <strstream>
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include "genError.h"

#include "ldrNode.h"
#include "ldrError.h"
#include "ldrSmtFlowchart.h"
#include "lde-flowchart.h"
#include "smt.h"
#include <messages.h>

extern ldrChartNodePtr build_stmt (smtTreePtr , int);
extern ldrChartNodePtr build_class(smtTreePtr , int);
extern ldrChartNodePtr build_chain(ldrChartNodePtr , int);
extern ldrChartNodePtr build_switch(smtTreePtr, int);  
extern ldrChartNodePtr build_loop_ldr(smtTreePtr, int);
extern ldrChartNodePtr build_block_ldr(smtTreePtr, int, 
       ldrChartNodePtr ldr_node = NULL, ldrRole l_role = ldrUNCLEAR);
extern ldrChartNodePtr build_simple_node(smtTreePtr, int,
                       ldrSymbolType, ldrRole role = ldrSIMPLE);
extern void reset_level( ldrNodePtr ldr_root);
extern ldrChartNodePtr build_goto(smtTreePtr, int level);

static ldrChartNodePtr build_file(smtTreePtr, int);
static ldrChartNodePtr build_fdecl(smtTreePtr, int);

static ldrChartNodePtr build_if_ldr(smtTreePtr, int);
static void  build_if_clause (ldrNodePtr &,  ldrChartNodePtr &,
                              ldrSymbolNodePtr, smtTreePtr, int);
static void build_fake_else (ldrNodePtr &, ldrChartNodePtr &,
                             ldrSymbolNodePtr, int);

extern void smt_get_node_type(smtTreePtr smt_root, int smt_type, objSet &);

extern ldrConnectionNodePtr
       connect1_1(ldrSymbolNodePtr, ldrSymbolNodePtr,
	          ldrConnectorType,  int level, int itself = 0);


// Top Level flowchart builder. It either dispatches to type
// specific builder or, if none is found, returns NULL.


ldrChartNodePtr lde_build_smt_flowchart ( smtTreePtr root, int level)
                           
{

   Initialize(lde_build_smt_flowchart);

   Assert(root);

   ldrChartNodePtr      new_ldr = NULL;
   int is_stmt = 0;

   switch (root->type) {

            case SMT_fdecl:
	    case SMT_fdef:
                 new_ldr = build_fdecl(root, level);
                 break;

            case SMT_if :  
                 new_ldr = build_if_ldr (root, level);
                 is_stmt = 1;
                 break;

            case SMT_nstdloop:
            case SMT_for:
            case SMT_while:
            case SMT_do:
            case SMT_dowhile:
                 new_ldr = build_loop_ldr (root, level);
                 is_stmt = 1;
                 break;

            case SMT_block:
                 new_ldr = build_block_ldr(root, level);
                 is_stmt = 1;
                 break;

//          simple nodes all treated the same

            case SMT_break:
                 new_ldr = build_simple_node(root, level,
                                 ldrBULLET, ldrBREAK);
                 break;

            case SMT_label:
                 new_ldr = build_simple_node(root, level, 
                                 ldrFLOW_LABEL, ldrLABEL);
                 break;

            case SMT_goto:
                 new_ldr = build_goto(root, level);
                 break;

            case SMT_file:
                 new_ldr = build_file(root, level);
                 break;

            case SMT_switch:
                 new_ldr = build_switch(root, level);
                 break;

//          processecing class don't ignore definitions etc.
//          unlike all other cases

            case SMT_cdecl:
                 new_ldr = build_class(root, level);
                 break;

//          Note, that normally decl nodes and some others will 
//                be skipped by get_next_smt.
//          Don't process those :

            case SMT_comment:
            case SMT_null:
            case SMT_token:
            case SMT_group:
                 break;

            case SMT_stmt:
            case SMT_macrostmt:
            case SMT_macrocall:
                 new_ldr = build_stmt (root, level);
                 break;

            default:

/*          case SMT_expr:
            case SMT_return:
            case SMT_continue:  
*/
                 new_ldr = build_simple_node(root, level, 
                                 ldrBULLET, ldrSIMPLE);
   }
   if(is_stmt && !new_ldr)
                 new_ldr = build_stmt (root, level);       
   return new_ldr;
}


// ------------------  FUNCTION declaration  -------------------------

//  This is a top of the ldr tree; if this node does not
//  contain the body it is function definition and either skipped
//  or processed as a simple statement (inside the class body).
//  
//  the input smt tree is :    smt_fdef 
//                                |
//                             smt_fdecl -> .* -> smt_block (with all stmts.)
//                                |
//                             smt_declspec (output type) -> smt_title (name + args.)

ldrChartNodePtr build_fdecl(smtTreePtr smt_fdef, int level)
{
   ldrChartNodePtr  ldr_fdecl = NULL;
   ldrChartNodePtr  ldr_fdef = NULL;
   ldrChartNodePtr  ldr_block;
   smtTreePtr       fdecl_block;
   smtTreePtr       smt_fdecl;

//------  Executable code ----------------------

   Initialize(build_fdecl);

   if (smt_fdef->type != SMT_fdef)                // new SMT structure - not implemented yet
       return( ldr_fdecl );

//     must be a method in CLASS
//
// if top is smt_fdef get fdecl

   ldr_fdef = db_new (ldrChartNode, (ldrFLOW_EMPTY));
   ldr_fdef->set_appTree(smt_fdef);
   smt_fdecl = smt_fdef->get_first();

// process smt_block with all stmnts inside


   // keep moving right from fdecl to find SMT_block
   fdecl_block = get_next_smt(smt_fdecl);
   while ( fdecl_block != NULL && fdecl_block->type != SMT_block )
     fdecl_block = get_next_smt(fdecl_block);

   if ( fdecl_block != NULL && fdecl_block->type == SMT_block ){

//         get body; if this is just a declaration - skip it

           ldr_fdecl = build_simple_node(smt_fdecl, level,
                                      ldrOVAL, ldrFDECL);
           if (ldr_fdecl != NULL)
               ldr_fdef->put_first(ldr_fdecl);

//         build func body if exists 

           ldr_block = build_block_ldr (fdecl_block, level+1);
           ldr_fdecl->put_after(ldr_block);

//-------- build connector between title and body

           ldrConnectionNodePtr fdecl_connect = 
                   connect1_1(ldr_fdecl, ldr_block, 
                              ldrcTRUE_BRANCH, level+1);
           ldr_block->put_after(fdecl_connect);
   }
   return(ldr_fdef);
}






// ------------------  BLOCK ------------------------------
//
// builds a block around either chain of app_nodes for
//          tree of appTree nodes starting from smt_block;
//          in last case  input ldr_node = NULL;  if ldr role is
//          supplied, set it to root

ldrChartNodePtr build_block_ldr(smtTreePtr smt_block, int level,
                ldrChartNodePtr ldr_node, ldrRole l_role)
{
   ldrSymbolNodePtr exit_symbol   = db_new(ldrSymbolNode, (ldrFLOW_EMPTY));
   ldrSymbolNodePtr entry_symbol  = db_new(ldrSymbolNode, (ldrENTRY_SYMBOL));

   ldrContainerNodePtr the_box  = db_new(ldrContainerNode,());
   ldrChartNodePtr     root     = db_new (ldrChartNode,
                                                (ldrFLOW_EMPTY));
   ldrSymbolNodePtr    prev_child = entry_symbol;
   ldrConnectionNodePtr connect_node;
   int                 skip_decl;     // flag = 0, if not skipping

//------  Executable code ----------------------

   Initialize(build_block_ldr);
   
   the_box->set_graphlevel(level);
   the_box->set_role(ldrCONTAINMENT);
   entry_symbol->set_role(ldrENTRY);
   entry_symbol->set_graphlevel(level+1);
   exit_symbol->set_role(ldrEXIT);
   exit_symbol->set_graphlevel(level+1);
   
   root->set_entry_symbol(entry_symbol);
   root->set_exit_symbol(exit_symbol);
   root->set_graphlevel(level);
   root->put_first(the_box);
   root->set_role(l_role);

   the_box->put_after(entry_symbol);

// if processing CLASS node don't skip declarations of smt

   if ( smt_block->type == SMT_cbody )
       skip_decl = 0;
   else
       skip_decl = 1;

   if (ldr_node == NULL)
   {

//    dive inside smt block; no existing ldr node is given

      smtTreePtr smt_child = smt_block->get_first();
      while( smt_child != NULL )
      {
          ldrChartNodePtr curr_child = 
              lde_build_smt_flowchart(smt_child, level+1);
          if (curr_child != NULL){
              prev_child->put_after(curr_child);
//            connect nodes

              if (prev_child->get_role() != ldrENTRY &&
                  skip_decl != 0 ){
                  connect_node = connect1_1(prev_child,
                           curr_child, ldrcSEQUENCE, level + 1);
                  curr_child->put_after(connect_node);
              }
              prev_child = curr_child;
          }
          smt_child = get_next_smt(smt_child, skip_decl);
       }
       prev_child->put_after(exit_symbol);
   }

// build block around chain of smt nodes starting from
// existing ldr node

   else{          
       reset_level(ldr_node);      // reset all levels (+1)

       entry_symbol->put_after(ldr_node);
       ldrChartNodePtr ldr_last = build_chain(ldr_node, level+1);
       if (ldr_last == NULL)
           ldr_last = ldr_node;
       ldr_last->put_after(exit_symbol);
   }

   root->set_appTree(smt_block);
   return(root);
}

// ------------------  RESET LEVEL ------------------------------
//  - walks down the tree iincrementing all levels by 1

void reset_level( ldrNodePtr ldr_root)
{
       Initialize(reset_level);

       ldrNodePtr ldr_curr;
       int lev = ldr_root->get_graphlevel() + 1;
       ldr_root->set_graphlevel(lev);
       ldr_curr = checked_cast(ldrNode, ldr_root->get_first());
       while (ldr_curr){
          reset_level(ldr_curr);
          ldr_curr = checked_cast(ldrNode,ldr_curr->get_next());
       }
}


// ------------------  SIMPLE NODE ------------------------------


ldrChartNodePtr build_simple_node(smtTreePtr smt_node, int level,
                ldrSymbolType sym_type, ldrRole ldr_role)
{
   ldrSymbolNodePtr new_symbol;
   ldrChartNodePtr  new_node;

//------  Executable code ----------------------

   Initialize(build_simple_node);
   
   new_node   = db_new(ldrChartNode, (ldrSOMETHING));
   if (smt_node != NULL)   
       new_node->set_appTree(smt_node);    

   new_node->set_graphlevel(level);
   if (ldr_role != ldrUNCLEAR)
       new_node->set_role(ldr_role);

   if (sym_type != ldrNOSYMBOL){
       if (sym_type == ldrFUNC_CALL){
           ldrFuncCallNodePtr f_symbol = db_new(ldrFuncCallNode,
                                  (sym_type, smt_node));
           f_symbol->set_graphlevel(level);
           f_symbol->set_role(ldr_role);
           new_node->set_exit_symbol(f_symbol);
           new_node->set_entry_symbol(f_symbol);
           new_node->put_first(f_symbol);
       }
       else{
           if (sym_type == ldrSOMETHING)
               new_symbol = db_new(ldrSymbolNode, ());

           else
               new_symbol = db_new(ldrSymbolNode, (sym_type));

           new_symbol->set_graphlevel(level);
           new_symbol->set_role(ldr_role);
           new_node->set_exit_symbol(new_symbol);
           new_node->set_entry_symbol(new_symbol);
           new_node->put_first(new_symbol);
       }
   }
   return(new_node);
}


// ------------------  STATEMENT processing  -------------------------
//
// Calls build_simple but first assign the role if this is a member
// protection type (like public, private, ..)in class definition.

ldrChartNodePtr build_stmt(smtTreePtr smt_stmt, int level)
{
   ldrChartNodePtr  ldr_node = NULL;
   ldrRole          ldr_role;
   char             *smt_data;
   smtTreePtr       smt_token;
   smtTreePtr       smt_parent;
   ostrstream       buff;

//------  Executable code ----------------------

   Initialize(build_stmt);

   ldr_role = ldrSIMPLE;
   smt_parent = smt_stmt->get_parent();

   if (smt_parent->type == SMT_cbody){
       smt_token = smt_stmt->get_first_leaf();
       smt_token->send_string(buff);
       buff << ends;
       smt_data = buff.str();
       if ( !strncmp(smt_data, "private", 7) ||
            !strncmp(smt_data, "public", 6)  ||
            !strncmp(smt_data, "protected", 9) )

       ldr_role = ldrCLASS_MEMBER_TYPE;
   }   

   ldr_node = build_simple_node(smt_stmt, level,
                     (ldrSymbolType )ldrBULLET, ldr_role);

   return(ldr_node);
}




// ------------------  IF (ELSE) STATEMENT ----------------------


ldrChartNodePtr build_if_ldr(smtTreePtr smt_root, int level)
{

//------  Executable code ----------------------

  Initialize(build_if_ldr);

   smtTreePtr smt_clause = smt_root->get_first();
   if (smt_clause == NULL || smt_clause->type != SMT_then_clause)
          return NULL;

   ldrNodePtr           ldr_prev;
   ldrChartNodePtr      ldr_prev_cond;
   ldrChartNodePtr      ldr_if_test;

     ldrSymbolNodePtr exit_symbol = db_new(ldrSymbolNode,(ldrEXIT_SYMBOL));
   exit_symbol->set_role(ldrEXIT);
   exit_symbol->set_graphlevel(level+1);

   ldrChartNodePtr if_root = db_new (ldrChartNode, (ldrIF));
   if_root->set_exit_symbol(exit_symbol);
   if_root->set_appTree(smt_root);
   if_root->set_graphlevel(level);

// first child is IF condition clause

// build ldr nodes for the then/ else_if /else clauses
//       - for each set role, attach to previous clause,
//         assign symbol, output last node in clause (ldr_prev)
//  Note, all connectors are put after the total if_exit since 
//  they require to have all symbol nodes processed before

   ldr_prev = if_root;
   ldr_prev_cond = NULL;
   int i;
   for (i = 0; i > -1; i++){

//    build chart nodes for condition and branch statements

      if (smt_clause->type == SMT_then_clause ||
          smt_clause->type == SMT_else_clause ||          
          smt_clause->type == SMT_else_if_clause )
          build_if_clause (ldr_prev, ldr_prev_cond, 
                           exit_symbol, smt_clause, level);

      smt_clause = get_next_smt(smt_clause);
      if (smt_clause == NULL)
          break;
   }

// if no ELSE exists - still build a connector using FLOW_EMPTY node

   if (i == 0)
       build_fake_else(ldr_prev, ldr_prev_cond, 
                       exit_symbol, level + 1);

   ldr_if_test = (ldrChartNodePtr) if_root->get_first();
   ldr_if_test->share_entry(if_root);
   
   ldr_prev->put_after(exit_symbol);   
   exit_symbol->splice();            // move connectors one level up
   ldr_prev->put_after(exit_symbol);   // since splice removed exit

   return(if_root);
}



// ------------------ CLAUSE -  IF/ELSE BRANCH  ------------------------

//
// build chart node for if, else, or else_if condition,
// and a chart node box for the statements,
// connect condition node to ldr_prev; 
// if index = 0, condition is if_test and the box is a true branch;
// build connector nodes and attach them as children to the exit

 void build_if_clause (ldrNodePtr &ldr_prev, ldrChartNodePtr &ldr_prev_cond,
      ldrSymbolNodePtr if_exit, smtTreePtr smt_clause, int level)
{
  ldrChartNodePtr ldr_branch;            // true branch node
  ldrChartNodePtr ldr_body;              // and a box around it
  ldrChartNodePtr ldr_cond;              // condition node (if/else)
  ldrNodePtr      branch_box;
  smtTreePtr      smt_cond;
  smtTreePtr      smt_branch;
  int             this_level;
  ldrRole         ldr_role;
  ldrConnectorType     end_conn_type;
  ldrConnectionNodePtr if_then_connect,
                       if_else_connect,
                       end_if_connect;
  ldrChartNodePtr ldr_yes;               // to place "YES" text
  ldrSymbolType   sym_type = ldrIF_FLOW;

//------  Executable code ----------------------

  Initialize(build_if_clause);

  smt_cond = smt_clause->get_first();

  if (ldr_prev_cond == NULL){                  // if test clause
      this_level = level;
      ldr_role = ldrFLOW_IF_TEST;
      end_conn_type = ldrcENDIF;
  }
  else{
      this_level = level + 1;
      if ( smt_clause->type == SMT_else_clause ){
              ldr_role = ldrLAST_ELSE;
              sym_type = ldrFLOW_EMPTY;
      }
      else
          ldr_role = ldrFLOW_ELSE;
      end_conn_type = ldrcENDELSE;
  }

// process clause first child - if/else condition
// - same symbol ldrIF_FLOW for if and else

  ldr_cond = build_simple_node(smt_cond, this_level, 
                               sym_type, ldr_role); 
  if (ldr_prev_cond == NULL)
      ldr_prev->put_first(ldr_cond);
  else
      ldr_prev->put_after(ldr_cond);

// process clause's next child - branch statements (can be 
// either a block or one executable stmt, but with possible comments)

  smt_branch = get_next_smt(smt_cond);
  ldr_body   = lde_build_smt_flowchart(smt_branch, this_level + 1);

  if (!ldr_body) {
     msg("STATUS: Missing conditional information - flowchart may be incomplete") << eom;
     return;
  }

// if branch is a box all done

  branch_box = checked_cast(ldrNode, ldr_body->get_first());
  if ( ! branch_box->ldrIsA(ldrContainerNodeType) )
      ldr_branch = build_block_ldr(smt_branch, this_level + 1,
                                   ldr_body);
  else
      ldr_branch = ldr_body;

  ldr_cond->put_after(ldr_branch);


// put "YES" text creating fake nodes

  if (ldr_role != ldrLAST_ELSE){
      ldr_yes = build_simple_node(NULL, this_level + 1,
                                  ldrFLOW_YES, ldrIF_YES);
      ldr_cond->put_after(ldr_yes);
  }

//-------------------- build connectors  and put after total exit

  if_then_connect = connect1_1(ldr_cond, ldr_branch,
                        ldrcTRUE_BRANCH, this_level+1);
  if_exit->put_first(if_then_connect);

  end_if_connect = connect1_1(ldr_branch, if_exit, 
                              end_conn_type, this_level+1);
  if_exit->put_first(end_if_connect);

  if (ldr_prev_cond != NULL){         // if-else or else_if - else
      if_else_connect = connect1_1(ldr_prev_cond, ldr_cond,
                                   ldrcIF_ELSE, this_level+1);
      if_exit->put_first(if_else_connect);
  }
  ldr_prev      = ldr_branch;
  ldr_prev_cond = ldr_cond;

  return;
}


// ------------------ BUILD_FAKE_ELSE ------------------------
 
//
// build a chart node for absent else; instead of else condition
// and a branch body there will be two FLOW_EMPTY nodes for the
// routing purposes; connectors will be of the same type.


 void build_fake_else (ldrNodePtr &ldr_prev,         // true branch
                       ldrChartNodePtr &ldr_prev_cond,    // if
                       ldrSymbolNodePtr if_exit, int level)
{
  ldrChartNodePtr      ldr_branch;         // fake branch node
  ldrChartNodePtr      ldr_cond;           // fake condition node 
  ldrConnectionNodePtr cond_branch_connect,
                       if_else_connect,
                       end_else_connect;

//------  Executable code ----------------------

  Initialize(build_fake_else);

//--- build fake else condition

  ldr_cond = build_simple_node(NULL, level, 
                              ldrFLOW_EMPTY, ldrLAST_ELSE);
  ldr_prev->put_after(ldr_cond);

//--- build fake branch statements

  ldr_branch = build_simple_node(NULL, level + 1,
                              ldrFLOW_EMPTY, ldrFAKE_LABEL);
  ldr_cond->put_after(ldr_branch);

//------------ build connectors  and put after total exit

  cond_branch_connect = connect1_1(ldr_cond, ldr_branch,
                        ldrcTRUE_BRANCH, level+1);
  if_exit->put_first(cond_branch_connect);

  end_else_connect = connect1_1(ldr_branch, if_exit, 
                                ldrcENDELSE, level+1);
  if_exit->put_first(end_else_connect);
  if_else_connect = connect1_1(ldr_prev_cond, ldr_cond,
                               ldrcIF_ELSE, level+1);
  if_exit->put_first(if_else_connect);
  ldr_prev      = ldr_branch;

  return;
}

// 
// ------------------  FILE ------------------------------
//
// builds   ldr for a file that may contain fdecls, classes , etc.;
// creates a box for each fdef and cdecl.

ldrChartNodePtr build_file(smtTreePtr smt_file, int level)
{
   ldrChartNodePtr   ldr_root  = db_new (ldrChartNode, (ldrFLOW_EMPTY));
   ldrChartNodePtr   ldr_prev;
   ldrChartNodePtr   ldr_node;

//------  Executable code ----------------------

   Initialize(build_file);
   
   ldr_prev = NULL;

// dive inside file
   
   if (smt_file == NULL)
       return(NULL);

   ldr_root->set_appTree(smt_file);
   smtTreePtr smt_node = smt_file->get_first();

   while( smt_node != NULL )
   {
//    filter smt nodes processing only FDEF and CLASS types

      int smt_type = smt_node->type;
      if (smt_type == SMT_fdef || smt_type == SMT_cdecl){
          ldr_node = lde_build_smt_flowchart(smt_node, level+1);
          if (ldr_node != NULL){            // skip comments, etc.
              if (ldr_prev == NULL)
                  ldr_root->put_first(ldr_node);
              else
                  ldr_prev->put_after(ldr_node);
              ldr_prev = ldr_node;
          }
       }
       smt_node = smt_node->get_next();
   }
   return(ldr_root);
}


// returns the last build ldr

ldrChartNodePtr build_chain(ldrChartNodePtr ldr_prev, int level)
{
   ldrChartNodePtr   ldr_node = NULL;
   smtTreePtr        smt_start;
   ldrConnectionNodePtr connect_node;

//------  Executable code ----------------------

   Initialize(build_chain);
   if (ldr_prev == NULL)
       Error(ERR_INPUT);
   
   smt_start = checked_cast(smtTree, ldrtree_get_apptree(ldr_prev));
//   smt_start = checked_cast(smtTree, get_relation(apptree_of_ldrtree,
//                                         ldr_prev));
// go along smt chain; build ldr nodes and link them together

   smtTreePtr smt_node = get_next_smt(smt_start);
   while( smt_node != NULL )
   {
          ldr_node = lde_build_smt_flowchart(smt_node, level+1);
          if (ldr_node != NULL){            // skip empty
              ldr_prev->put_after(ldr_node);

//            connect the nodes

              if (ldr_prev->get_role() != ldrENTRY){
                  connect_node = connect1_1(ldr_prev, ldr_node,
                                       ldrcSEQUENCE, level + 1);
                 ldr_node->put_after(connect_node);
              }
              ldr_prev = ldr_node;
          }
          smt_node = get_next_smt(smt_node);
   }

   return(ldr_prev);
}


///////////////////////////////// Building Connectors: /////////////////

// if itself argument = 1, connection is done to the 
//    node itself not to entry/exit symbols. This allows, for
//    example, to connect boxes at the side.

ldrConnectionNodePtr	
connect1_1(ldrSymbolNodePtr inp_node, ldrSymbolNodePtr outp_node,
	   ldrConnectorType con_type, int level, int itself)
{
   ldrConnectionNodePtr me = db_new(ldrConnectionNode, (con_type));
   me->set_symbol_type(con_type);
   if (itself == 0){
       me->add_input(inp_node->get_exit_symbol());
       me->add_output(outp_node->get_entry_symbol());
   }
   else{
       me->add_input(inp_node);
       me->add_output(outp_node);
   }
   me->set_graphlevel(level);
   return(me);
}

/* 
ldrConnectionNodePtr
connect2_1(ldrSymbolNodePtr inp_node1, ldrSymbolNodePtr inp_node2,
	   ldrSymbolNodePtr outp_node, ldrConnectorType con_type,
	   int level)
{
   ldrConnectionNodePtr me = db_new(ldrConnectionNode, (con_type));
   me->add_input(inp_node1->get_exit_symbol());
   me->add_input(inp_node2->get_exit_symbol());
   me->add_output(outp_node->get_entry_symbol());
   me->set_graphlevel(level);
   return(me);
}

*/


// GET NEXT and skip unprocessing stmts ( comments, decls, etc.)
// if skip_flag == 1, which is default.

smtTreePtr get_next_smt( smtTreePtr smt_node, int skip_flag)
{
  Initialize(get_next_smt);

  smtTreePtr smt_next = smt_node;
  int skip;

  do {
    smt_next =  smt_next->get_next();
    if (smt_next == NULL)
      break;
    skip = 0;     
    int type = smt_next->type;
    if(
       type == SMT_comment      ||
       type == SMT_m_include    ||
       type == SMT_m_if         ||
       type == SMT_m_else       ||
       type == SMT_m_endif      ||
       type == SMT_m_define     ||
       type == SMT_token
       ) skip = 1;
    else if(skip_flag) if(
       type == SMT_decl         ||
       type == SMT_pdecl        ||
       type == SMT_gdecl        ||
       type == SMT_edecl        ||
       type == SMT_ldecl        ||
       type == SMT_list_decl    ||
       type == SMT_list_pdecl
       ) skip = 1;

  }   while (skip);

  return smt_next;
}

void lde_detach_flowchart(ldrChartNodePtr old_node)
{
   ldrConnectionNodePtr input_connector, output_connector;
   ldrSymbolNodePtr output_output;
   
   Initialize(lde_detach_flowchart);
   output_connector =
     checked_cast(ldrConnectionNode, old_node->get_output_connectors());
   if (((Obj*)output_connector)->collectionp())
     Error(CANNOT_DISCONNECT);
   if ((output_connector->get_symbol_type()) != ldrcSEQUENCE)
     Error(CANNOT_DISCONNECT);
   input_connector =
     checked_cast(ldrConnectionNode, old_node->get_input_connectors());
   output_output =
     checked_cast(ldrSymbolNode, output_connector->get_output_symbols());
   input_connector->replace_output(checked_cast(ldrSymbolNode,old_node), output_output);
   output_connector->remove_from_tree();
   return;
}

/*
   START-LOG-------------------------------------------

   $Log: lde_smt_flowchart.cxx  $
   Revision 1.9 2000/07/12 18:10:19EDT ktrans 
   merge from stream_message branch
Revision 1.2.1.8  1994/02/16  13:56:45  mg
Bug track: 1
skip #if...

Revision 1.2.1.7  1994/02/11  16:09:28  mg
Bug track: 6237
Added stmt to handle mutilated if-then inside macros for flowcharts

Revision 1.2.1.6  1993/10/14  18:52:30  kws
Bug track: 4929
Fix crash in flowchart conditional extraction when mapping is incomplete

Revision 1.2.1.5  1993/04/16  22:58:48  sergey
Added support for macros in build_if_ldr. FIxed bug #3397.

Revision 1.2.1.4  1992/12/23  20:19:04  sergey
Fixed a bug in reset_level; minor clean up.

Revision 1.2.1.3  1992/11/21  22:16:34  builder
typesafe casts.

....

Revision 1.2.1.2  1992/10/09  19:19:01  sergey
Initial Version.



   END-LOG---------------------------------------------

*/
