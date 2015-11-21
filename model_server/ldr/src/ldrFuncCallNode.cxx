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
// ldrFuncCallNode.h.C 
// synopsis : Methods of ldrFuncCallNode class.
//
// description: 
//

#include "objRelation.h"
#include "objOper.h"
#include "objTree.h"
#include "ldrNode.h"
#include "ddict.h"
#include "symbolLdrHeader.h"


#include <machdep.h>

//============================================== ldrFuncCallNode:

ldrFuncCallNode::ldrFuncCallNode(ldrSymbolType sym,
                                 appTreePtr app_ptr)
      : ldrSymbolNode(ldrSymbolType(sym)) 
{
   Initialize(ldrFuncCallNode::ldrFuncCallNode);

   ddElementPtr dd_el;

   if ( is_smtTree(app_ptr) )
      dd_el = smt_get_dd( checked_cast(smtTree,app_ptr));
   else if ( is_ddElement(app_ptr) )
      dd_el = checked_cast(ddElement, app_ptr);
   else
      dd_el = checked_cast(ddElement,app_ptr);

   int str_len =strlen(dd_el->get_name()) + 1;
   name.put_value(dd_el->get_name(), str_len);

// display or hide args according to ldr_hdr 

//   fileLanguage curr_lang = (fileLanguage ) dd_el->get_xrefSymbol()->get_language();
//   if ( curr_lang == FILE_LANGUAGE_CPP ) {

   symbolLdrHeader* hd = checked_cast (symbolLdrHeader, get_header ());
   this->modif_arg(name, hd->get_disp_arg() );

//   put_relation(apptree_of_ldrtree, this, dd_el);

//  set collapse flags

   top_end    = 0;
   bottom_end = 0;
   upper_tree = 0;
   disp_arg   = -1;
   func_type  = 0;
}

ldrFuncCallNode::ldrFuncCallNode(ldrSymbolType sym, const char* nm)
: ldrSymbolNode(ldrSymbolType(sym)) , name (nm), top_end (0), bottom_end (0),
upper_tree (0), disp_arg(-1), func_type(0)
{}

void ldrFuncCallNode::send_string(ostream& os) const {
   Initialize(ldrFuncCallNode::send_string);

   os << (char*) name;

   return;
}


int ldrFuncCallNode::ldrIsA(ldrNodeTypes type) const {
   return type == ldrFuncCallNodeType;
}

objArr *ldrFuncCallNode::get_tok_list(int ) {
    return NULL;
}


void ldrFuncCallNode::modif_arg(char *full_name, int arg_disp_mode) {

    Initialize (ldrFuncCallNode::modif_arg);

    if ( disp_arg != -1 )                           // node has its own flag which 
         arg_disp_mode = disp_arg;                  // overwrites the ldr_hdr`s one

    if ( arg_disp_mode )

//      add args
        name.put_value((const char *)full_name);

    else {

//      strip args of the root and all its children functions

        char *end_name_ptr = OS_dependent::index( full_name, '(');
        if (end_name_ptr != NULL)                   // no args - skip it

//          second arg below is new name length 
            name.put_value( full_name, end_name_ptr - full_name );
    }
}


/*
   START-LOG-------------------------------------------

   $Log: ldrFuncCallNode.cxx  $
   Revision 1.3 1996/03/01 10:22:25EST aharlap 
   stage dd cleanup
Revision 1.2.1.8  1993/12/16  13:49:20  azaparov
Bug track: 4418
Fixed bug 4418

Revision 1.2.1.7  1993/07/20  18:23:50  harry
HP/Solaris Port

Revision 1.2.1.6  1993/05/25  13:58:46  sergey
Added code to support display/hide arguments on single tree node. Part of bug #3319.

Revision 1.2.1.5  1993/04/14  18:43:06  sergey
Added show/hide args support for C++ call tree. Fixed bug #3305.

Revision 1.2.1.4  1992/12/21  23:05:34  aharlap
modified for new xref

Revision 1.2.1.3  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:43:50  aharlap/sergey
Initial Revision.


   END-LOG---------------------------------------------

*/
