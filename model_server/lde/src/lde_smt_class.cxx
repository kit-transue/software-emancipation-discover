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
// lde_smt_class.C
//
// synopsis : Creates ldr nodes for class smt object.
//
// description : Class smt node (SMT_cdecl type) is a parent of class
//               title node and a class body with class definition.
//               Class body looks like a regular smt_block.
//               Inline functions bodies are not exploded and only
//               their declarations are shown.
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


extern ldrChartNodePtr build_block_ldr(smtTreePtr, int, 
       ldrChartNodePtr ldr_node = NULL, ldrRole l_role = ldrUNCLEAR);

extern ldrChartNodePtr build_simple_node(smtTreePtr, int,
               ldrSymbolType, ldrRole role = ldrSIMPLE);


extern ldrConnectionNodePtr
       connect1_1(ldrSymbolNodePtr, ldrSymbolNodePtr,
	          ldrConnectorType,  int level, int itself = 0);



// ------------------  CLASS STATEMENT ----------------------


ldrChartNodePtr build_class(smtTreePtr smt_root, int level)
{
   ldrChartNodePtr    ldr_class_root = NULL;
   ldrChartNodePtr    ldr_body;
   ldrChartNodePtr    ldr_class_name;
   ldrSymbolNodePtr   exit_symbol = db_new(ldrSymbolNode,
                                           (ldrFLOW_EMPTY));
   smtTreePtr         smt_class_body,
                      smt_class_name;    // class name
   ldrConnectionNodePtr   ldr_connect;

//------  Executable code ----------------------

   Initialize(build_class);

// process class name
// if this is just a declaration of  Class A; type - skip it.

   smt_class_name = checked_cast(smtTree, smt_root->get_first());
   smt_class_body = get_next_smt(smt_class_name);
   if (smt_class_body == NULL || smt_class_body->type != SMT_cbody)
       return NULL;

   ldr_class_root = db_new(ldrChartNode, (ldrSOMETHING));
   exit_symbol->set_role(ldrEXIT);
   exit_symbol->set_graphlevel(level+1);

   ldr_class_root->set_exit_symbol(exit_symbol);
   ldr_class_root->set_appTree(smt_root);

   ldr_class_name = build_simple_node(smt_class_name, level, 
                                   ldrBULLET, ldrCLASS); 
   ldr_class_root->put_first(ldr_class_name);

// process member data and methods groupping them by 
// protection type (public/private/protected)

   ldr_body = build_block_ldr (smt_class_body, level+1);
   ldr_class_name->put_after(ldr_body);

//-------------------- build connector

// class_name -> class_body

   ldr_connect = connect1_1(ldr_class_name, ldr_body,
                            ldrcSEQUENCE, level+1);
    
   ldr_body->put_after(ldr_connect);

   ReturnValue(ldr_class_root);
}

/*
   START-LOG-------------------------------------------

   $Log: lde_smt_class.cxx  $
   Revision 1.2 2000/07/07 08:11:02EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.5  1993/11/09  01:38:05  boris
Bug track: #5204
Fixed flowchart crash

Revision 1.2.1.4  1993/04/01  18:24:20  sergey
Check for no body case. FIxed bug #3081

Revision 1.2.1.3  1992/11/21  22:16:34  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:19:00  sergey
Initial Revision.



   END-LOG---------------------------------------------

*/
