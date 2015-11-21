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

// lde_smt_goto.C
//
// synopsis : Creates ldr nodes for flowchart from smt objects.
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


extern ldrChartNodePtr build_simple_node(smtTreePtr, int,
               ldrSymbolType, ldrRole role = ldrSIMPLE);

extern ldrConnectionNodePtr
       connect1_1(ldrSymbolNodePtr, ldrSymbolNodePtr,
	          ldrConnectorType,  int level, int itself = 0);


// ---------------  GOTO  -------------------------

// Builds simple node with text goto inside a box and
// adds a connector. If the nodes are close (in one block)
// actually connect them, otherwise just show arrow from
// goto.


ldrChartNodePtr build_goto(smtTreePtr smt_goto, int level)
{
   ldrChartNodePtr  ldr_goto;
   ldrSymbolNodePtr fake_label;
   ldrConnectionNodePtr goto_connect;

   Initialize(build_goto);

// ldr chart node is ldr_goto

   ldr_goto = build_simple_node(smt_goto, level, ldrBULLET,
                                ldrGOTO);

// if nodes are remote build a fake empty symbol near goto
// and a connector to it to show something

   fake_label = build_simple_node(NULL, level + 1,
                            ldrFLOW_EMPTY, ldrFAKE_LABEL);

// cannot put after ldr_goto since it does not have a parent yet

   ldrSymbolNodePtr ldr_goto_exit = ldr_goto->get_exit_symbol();
   ldr_goto_exit->put_after(fake_label);

// build connector from goto to fake_label

   goto_connect = connect1_1(ldr_goto, fake_label,
                        ldrcLEFT_HORIZONTAL, level+1);
   fake_label->put_after(goto_connect);

   ReturnValue(ldr_goto);
}

/*
   START-LOG-------------------------------------------

   $Log: lde_smt_goto.cxx  $
   Revision 1.2 2000/07/07 08:11:06EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.3  1992/11/21  22:16:34  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:19:03  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
