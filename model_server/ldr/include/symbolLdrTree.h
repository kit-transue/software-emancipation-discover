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
#ifndef __symbolLdrTree_h_
#define __symbolLdrTree_h_

#include "ldrNode.h"
#include "xref.h"

RelClass(symbolLdrTree);

class symbolLdrTree : public ldrChartNode {
   symbolPtr  xref;
   linkType    parent_link_type;

 public:
   unsigned top_end : 1;
   unsigned bottom_end : 1;

   symbolLdrTree (fsymbolPtr , linkType);
   define_relational(symbolLdrTree,ldrChartNode);
   symbolPtr  get_xref () const { return xref; }
   virtual void send_string (ostream &) const;
   virtual objArr* get_tok_list (int mode = 0);
   
   virtual int get_symbolPtr(symbolPtr*) const;

   int recursive;  // Non zero if this element is recursive in the Tree
};

generate_descriptor (symbolLdrTree, ldrChartNode);

#endif

/*
   START-LOG-------------------------------------------

   $Log: symbolLdrTree.h  $
   Revision 1.2 1994/11/14 19:10:20EST mg 
   Bug track: NA
   virtual int get_symbolPtr()
// Revision 1.3  1993/12/16  13:48:59  azaparov
// Bug track: 4418
// Fixed bug 4418
//
// Revision 1.2  1993/03/15  21:38:53  davea
// change xrefSymbol* to symbolPtr
//
// Revision 1.1  1992/12/21  23:09:23  aharlap
// Initial revision
//

   END-LOG---------------------------------------------

*/



