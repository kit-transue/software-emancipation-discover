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
// lde_smt_switch.C
//
// synopsis : Creates ldr nodes for switch / case smt objects.
//
// description : Switch is a block that has SMT_clause nodes at
//               the top level. Each one has SMT_case as the first
//               child and statements + break as next children.
//


#include "genError.h"
#include "genTable.h"
#include "ldrTypes.h"

#include "ldrNode.h"
#include "ldrApplTypes.h"
#include "ldrSmtFlowchart.h"
#include "ldrSmtHierarchy.h"
#include "lde-flowchart.h"
#include "smt.h"

extern ldrChartNodePtr get_case_break(ldrChartNodePtr);

extern ldrChartNodePtr build_block_ldr(smtTreePtr, int, 
       ldrChartNodePtr ldr_node = NULL, ldrRole l_role = ldrUNCLEAR);

extern ldrChartNodePtr build_simple_node(smtTreePtr, int,
               ldrSymbolType, ldrRole role = ldrSIMPLE);

extern void build_case (ldrChartNodePtr &, ldrSymbolNodePtr ,
                        smtTreePtr , ldrChartNodePtr &, int );

extern ldrConnectionNodePtr
       connect1_1(ldrSymbolNodePtr, ldrSymbolNodePtr,
	          ldrConnectorType,  int level, int itself = 0);

// ------------------  SWITCH STATEMENT ----------------------


ldrChartNodePtr build_switch(smtTreePtr smt_root, int level)
{
   ldrChartNodePtr    ldr_sw_root = 
                                 db_new(ldrChartNode, (ldrSOMETHING));
   ldrChartNodePtr    ldr_prev;
   ldrChartNodePtr    ldr_sw_test;
   ldrSymbolNodePtr   exit_symbol = db_new(ldrSymbolNode,
                                             (ldrEXIT_SYMBOL));
   smtTreePtr         smt_clause,
                      smt_block,
                      smt_sw_test;    // switch test
   ldrChartNodePtr    ldr_break;      // 0,if case does not have one.

//------  Executable code ----------------------

   Initialize(build_switch);

   exit_symbol->set_role(ldrEXIT);
   exit_symbol->set_graphlevel(level+1);

   ldr_sw_root->set_exit_symbol(exit_symbol);
   ldr_sw_root->set_appTree(smt_root);

// build switch condition symbol

   smt_sw_test = checked_cast(smtTree, smt_root->get_first());
   ldr_sw_test = build_simple_node(smt_sw_test, level, 
                                   ldrFLOW_SWITCH, ldrSWITCH); 
   ldr_sw_root->put_first(ldr_sw_test);

// process clause (case) smt nodes; skip smt_block

   smt_block = get_next_smt(smt_sw_test);
   if (smt_block->type != SMT_block)
       Error (ERR_INPUT);             // paranoja

   smt_clause = checked_cast(smtTree,smt_block->get_first());
   ldr_prev = ldr_sw_test;
   ldr_break = NULL;
   while ( smt_clause != NULL ){

//    build ldr nodes for the next clause (case / default),
//    and attach it to previous clause; output new ldr_prev

      if (smt_clause->type == SMT_case_clause)        
          build_case ( ldr_prev, exit_symbol, smt_clause,
                       ldr_break, level+1 );

      smt_clause =  get_next_smt(smt_clause );
   }

   ldr_sw_test = (ldrChartNodePtr) ldr_sw_root->get_first();
   ldr_sw_test->share_entry(ldr_sw_root);
   
   ldr_prev->put_after(exit_symbol);   
   exit_symbol->splice();            // move connectors one level up
   ldr_prev->put_after(exit_symbol);   // since splice removed exit

   ReturnValue(ldr_sw_root);
}



// ------------------ SWITCH CLAUSE (CASE or DEFAULT)  -------

//
// build ldr chart node for case statement and build an ldr block
// for case body statements; build fake node for default and a 
// block for the default body; connect condition node to ldr_prev; 

  void build_case (ldrChartNodePtr &ldr_prev, 
                ldrSymbolNodePtr switch_exit, smtTreePtr  smt_clause,
                ldrChartNodePtr &ldr_break, int level)
{
  ldrChartNodePtr ldr_case_body;         // case executable stmts
  ldrChartNodePtr ldr_body;              // and a box around it
  ldrChartNodePtr ldr_case;              // case stmt node
  ldrNodePtr      ldr_box;
  smtTreePtr      smt_case;
  smtTreePtr      smt_body;
  ldrConnectionNodePtr  case_exit_conn;
  ldrConnectionNodePtr  case_body_conn;
  ldrConnectionNodePtr  prev_case_conn;

//------  Executable code ----------------------

  Initialize(build_case);

// process clause first child - case statement

   smt_case = checked_cast(smtTree, smt_clause->get_first());
   ldr_case = build_simple_node(smt_case, level, 
                               ldrFLOW_CASE, ldrCASE);
   ldr_prev->put_after(ldr_case);

// process case's body - a set of statements

   smt_body =  get_next_smt(smt_case);
   if (smt_body == NULL)
       ldr_case_body = ldr_case;
   else{

//     get real body skipping new funny comments etc.

       while ( smt_body != NULL ){
          ldr_body = lde_build_smt_flowchart(smt_body, level+1);
          if (ldr_body != NULL)
              break;
          smt_body = get_next_smt(smt_body);          
       } 

//     check if the body is not empty and if it is a box already

       if (ldr_body == NULL)
           ldr_case_body = ldr_case;
       else{
           ldr_box = checked_cast(ldrNode, ldr_body->get_first());
           if ( ! ldr_box->ldrIsA(ldrContainerNodeType) )
               ldr_case_body = build_block_ldr(smt_body, level + 1,
                                           ldr_body);
           else
               ldr_case_body = ldr_body;
           ldr_case->put_after(ldr_case_body);
       }
   }

//-------------------- build connectors  and put after total exit

// prev_case -> case

   if (ldr_break == NULL){
      prev_case_conn = connect1_1(ldr_prev, ldr_case,
                                  ldrcTRUE_BRANCH, level+1);
      switch_exit->put_first(prev_case_conn);
   }

// case -> body

   if (ldr_case_body != ldr_case){
      case_body_conn = connect1_1(ldr_case, ldr_case_body,
                                  ldrcTRUE_BRANCH, level+1);
      switch_exit->put_first(case_body_conn);

      ldr_break = get_case_break(ldr_case_body);
      if (ldr_break != NULL){

//        case_body -> switch_exit

          case_exit_conn = connect1_1(ldr_break, switch_exit, 
                                  ldrcCASE_EXIT, level+1);
          switch_exit->put_first(case_exit_conn);
      }
   }

   if (smt_case->type == SMT_default){

//    default -> switch_exit

      case_exit_conn = connect1_1(ldr_case_body, switch_exit, 
                                  ldrcTRUE_BRANCH, level+1);
      switch_exit->put_first(case_exit_conn);
   }
   ldr_prev = ldr_case_body;

   Return;
}



// ------------------ GET BREAK in CASE  ------------------------
//
// returns ldr node for BREAK it exists or NULL

   ldrChartNodePtr  get_case_break(ldrChartNodePtr ldr_body)
{
   ldrChartNodePtr ldr_break;
   ldrNodePtr      ldr_curr;     // can be Container or Connector
   smtTreePtr      smt_node;

//------  Executable code ----------------------

   Initialize(get_case_break);

// ldr_body is always a block; check its children for BREAK
 
   ldr_break = NULL;
   ldr_curr = checked_cast(ldrNode,ldr_body->get_first()); 
   while( ldr_curr != NULL ){
          smt_node =
            checked_cast(smtTree,ldrtree_get_apptree(
                                              ldr_curr));
          if (smt_node != NULL &&
              smt_node->type == SMT_break){
              ldr_break =  (ldrChartNodePtr) ldr_curr;
              break;
          }
          ldr_curr = checked_cast(ldrNode,ldr_curr->get_next());
   }
 
   ReturnValue(ldr_break);
}

/*
   START-LOG-------------------------------------------

   $Log: lde_smt_switch.cxx  $
   Revision 1.3 2000/07/07 08:11:11EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.3  1992/11/21  22:16:34  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:19:07  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
