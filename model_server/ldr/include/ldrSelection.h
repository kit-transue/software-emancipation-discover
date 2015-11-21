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
#ifndef _ldrSelection_h_
#define _ldrSelection_h_

#ifndef _objOper_h
#include <objOper.h>
#endif

// ldrSelection is a representation in ldr world of a
// selected object or  an insertion point.  It can be
// mapped  to  application  node,  but  the   mapping
// functions depend on  the type of  the diagram, and
// for this reason they  are methods of corresponding
// ldr headers.
//
// A selected node is specified by ldr_symbol only --
// ldr_conector is  NULL.   Insertion    Point     is
// specified by both symbol and connector.
// The "subtype" is reserved for future use.
//
// CONSTRAINT:  ldr_symbol must  be  either input  or
// output of ldr_connector


RelClass(ldrSymbolNode);
RelClass(ldrConnectionNode);

class ldrSelection {
 public:
   ldrSymbolNodePtr     ldr_symbol;
   ldrConnectionNodePtr ldr_connector;
   int			subtype;

   ldrTreePtr ldr_node;
   int        offset;

   ldrSelection(ldrTreePtr = NULL, int = NULL);   
};

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrSelection.h  $
   Revision 1.2 1995/08/26 09:35:16EDT wmm 
   New group/subsystem implementation
 * Revision 1.2.1.3  1993/03/26  04:37:41  boris
 * Added Smod Short
 *
 * Revision 1.2.1.2  1992/10/09  19:50:28  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
