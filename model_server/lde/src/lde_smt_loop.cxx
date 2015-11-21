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
// lde_smt_loop.C
//
// synopsis : Creates ldr flowchart nodes for smt loops (for,while,do).
//

#include "genError.h"
#include "smt.h"
#include "lde-flowchart.h"


extern ldrChartNodePtr build_block_ldr(smtTreePtr, int, 
                       ldrChartNodePtr, ldrRole);
extern ldrChartNodePtr build_simple_node(smtTreePtr, int,
               ldrSymbolType, ldrRole role = ldrSIMPLE);

//--------------  WHILE / FOR / DO LOOPS ------------------

// loop_root --first-> title (or macro)   --next->body (box) --next->footer
//  (level)      (level + 1)                 (level+1)
// -->next exit
//   (level + 1)
// footer is optional and exists for do/while only

ldrChartNodePtr build_loop_ldr(smtTreePtr smt_loop, int level)
{
   ldrNodePtr          ldr_box;
   ldrChartNodePtr     ldr_first;
   ldrSymbolNodePtr    exit_symbol;
   ldrSymbolNodePtr    ldr_last; 

//------  Executable code ----------------------

   Initialize(build_loop_ldr);
   
// build title (init, condition, step) 

   smtTreePtr smt_title = smt_loop->get_first();
   while(smt_title && !get_next_smt(smt_title))
     smt_title = smt_title->get_first();

   if(!smt_title)
     return NULL;

   smtTree* smt_node = get_next_smt(smt_title);
   if(!smt_node)
     return NULL;
   
   ldrChartNodePtr loop_title  = build_simple_node(smt_title, level,
                                  ldrFLOW_LOOP, ldrLOOP_TITLE);
   if(!loop_title)
     return NULL;
   
// build loop body (if not a block put it in block)

   ldrChartNodePtr ldr_body = lde_build_smt_flowchart(smt_node, level + 2);
   if(!ldr_body){
     obj_delete(loop_title);
     return NULL;
   }

   ldrChartNodePtr  loop_root = db_new (ldrChartNode, (ldrLOOP));
   exit_symbol = db_new(ldrSymbolNode, (ldrFLOW_EMPTY));

   exit_symbol->set_role(ldrEXIT);
   exit_symbol->set_graphlevel(level+1);
   loop_root->set_exit_symbol(exit_symbol);


   loop_root->set_appTree(smt_loop); 
   loop_root->put_first(loop_title); 

   ldr_box = checked_cast(ldrNode, ldr_body->get_first());

   ldrChartNodePtr     loop_body;
   if ( !ldr_box->ldrIsA(ldrContainerNodeType) )
      loop_body = build_block_ldr(smt_node, level+1, ldr_body,
                                  ldrLOOP_BODY);
   else{
      loop_body = ldr_body;
      loop_body->set_role(ldrLOOP_BODY);
   }

   loop_title->put_after(loop_body);
   ldr_last = loop_body;

// build footer if any (do/while case only)

   smt_node =  get_next_smt(smt_node);
   if(smt_node != NULL) 
   {
      ldrChartNodePtr loop_footer = 
                build_simple_node(smt_node, level + 1,
                      ldrFLOW_LOOP, (ldrRole) ldrLOOP_FOOTER);

      ldr_last->put_after(loop_footer);
      ldr_last = loop_footer;
   }

   ldr_first = (ldrChartNodePtr) loop_root->get_first();
   ldr_first->share_entry(loop_root);
   ldr_last->put_after(exit_symbol);

//----------------- build connectors  and put after total exit
//  -- no connectors for loops

   return loop_root;
}

/*
   START-LOG-------------------------------------------
p
   $Log: lde_smt_loop.cxx  $
   Revision 1.2 1994/08/02 11:42:44EDT builder 
   
Revision 1.2.1.7  1994/08/01  19:13:48  mg
Bug track: NA
macro-generated loop has NULL body

Revision 1.2.1.6  1994/03/23  15:49:31  mg
Bug track: 1
flowchart for loop with macro

Revision 1.2.1.5  1993/05/19  00:37:39  sergey
Hide loop if inside macro. Fixed bug # 3147.

Revision 1.2.1.4  1992/12/24  20:36:03  sergey
Added SMT_macrocall case which fixes the bug #2153.

Revision 1.2.1.3  1992/11/21  22:16:34  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:19:05  sergey
Initial Version



   END-LOG---------------------------------------------

*/
