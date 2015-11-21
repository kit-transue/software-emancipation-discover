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
#ifndef _ldrHierarchy_h_
#define _ldrHierarchy_h_

#include "objRelation.h"
#include "OperPoint.h"
#include "ldrSelection.h"
#include "ldrHeader.h"

// This is a generic ldr header for all hierarchical diagrams.

class ldrHierarchy : public ldrHeader {
   
 public:
   ldrHierarchy() {};
   virtual void build_selection(const ldrSelection&, OperPoint&);
   define_relational(ldrHierarchy,ldrHeader);
};

generate_descriptor(ldrHierarchy,ldrHeader);

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrHierarchy.h  $
   Revision 1.3 1997/05/21 12:10:23EDT twight 
   Bug track: 14168
   Reviewed by: mb
   
   Changed parameter from 'ldrSelection& const' to 'const ldrSelection&'.
 * Revision 1.2.1.3  1993/02/17  20:50:27  kol
 * removed wrong const from arg build_selection
 *
 * Revision 1.2.1.2  1992/10/09  19:50:20  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
