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
#include <ddict.h>
#include "symbolLdrHeader.h"
#include "symbolLdrTree.h"

init_relational (symbolLdrTree, ldrChartNode);

symbolLdrTree::symbolLdrTree (fsymbolPtr  xr, linkType lt) 
: ldrChartNode(ldrSOMETHING), xref (xr), parent_link_type (lt), top_end (0),
  bottom_end (0), recursive (0)
{}
int symbolLdrTree::get_symbolPtr(symbolPtr*symp) const
{
  *symp=get_xref();
  return 1;
}
void symbolLdrTree::send_string (ostream &os) const
{
    Initialize (symbolLdrTree::send_string);

    Assert (xref->xrisnotnull());
    xref.print (os);
}

objArr* symbolLdrTree::get_tok_list (int)
{
    return NULL;
}

/*
   START-LOG-------------------------------------------

   $Log: symbolLdrTree.cxx  $
   Revision 1.4 1999/03/29 11:43:47EST Sudha Kallem (sudha) 
   changes to support new pmod format
// Revision 1.3  1993/12/16  13:50:01  azaparov
// Bug track: 4418
// Fixed bug 4418
//
// Revision 1.2  1993/03/15  21:52:35  davea
// change xrefSymbol* to fSymbolPtr
//
// Revision 1.1  1992/12/21  23:05:34  aharlap
// Initial revision
//

   END-LOG---------------------------------------------

*/
