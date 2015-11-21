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
#include "genError.h"
#include "OperPoint.h"
#include "ldrSelection.h"
#include "ldrError.h"
#include "ldrNode.h"
#include "ldrHierarchy.h"
#include "ddict.h"

// Translate ldr selection to Operation Point.  Operation //
// Point is expressed  in terms of  application node  and //
// direction (AFTER, BEFORE,  FIRST), while ldr selection //
// specifies an  edge of an ldr  connector -- the edge is //
// actually given by an input  or  an  output node of the //
// connector.

init_abstract_relational(ldrHierarchy,ldrHeader);

void ldrHierarchy::build_selection(const ldrSelection& olds , OperPoint&    app_point)
{
   Initialize(ldrHierarchy::build_selection);

   app_point.node = olds.ldr_symbol->get_appTree();

   if(!(int)app_point.node) {
      ldrNodePtr symbol_parent;
      symbol_parent = checked_cast(ldrNode, olds.ldr_symbol->get_parent());
      if (symbol_parent)
         app_point.node = symbol_parent->get_appTree();
   }
   IF(!(int)app_point.node)
     Error(BAD_LDR_SELECTION);

// if is ddElement, use corresponding smt node:

   if (is_ddElement(app_point.node)) {
      appTree* p = dd_get_smt(checked_cast(ddElement,app_point.node));
      if (p)
	 app_point.node = p;
   }

   if (olds.ldr_connector == NULL)
     app_point.type = FIRST;        // was REPLACE; -- SERGEY
   else if (olds.ldr_connector->is_input_p
	    (checked_cast(ldrSymbolNode, olds.ldr_symbol)))
     app_point.type = FIRST;
   else if (olds.ldr_connector->is_output_p
	    (checked_cast(ldrSymbolNode, olds.ldr_symbol)))
     app_point.type = AFTER;
   else Error(BAD_LDR_SELECTION);
   Return;
}

/*
   START-LOG-------------------------------------------

   $Log: ldrHierarchy.cxx  $
   Revision 1.3 1997/05/21 12:14:17EDT twight 
   Bug track: 14168
   Reviewed by: mb
   
   Changed parameter from 'ldrSelection& const' to 'const ldrSelection&'.
Revision 1.2.1.4  1992/12/21  03:10:16  aharlap
selection use symbolPtr

Revision 1.2.1.3  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:43:52  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
