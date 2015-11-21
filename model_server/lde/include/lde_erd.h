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
// lde_erd
//------------------------------------------
// Synopsis:
// declaration for Entity-Relation Diagram ldr extractor
//
// Description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------


#ifndef _lde_erd_h
#define _lde_erd_h

#include "ldrNode.h"
#include "oodt_ui_decls.h"

class Regexp;
RelClass(RTLNode);

extern ldrNodePtr lde_extract_erd(const symbolArr&, int, Regexp*, bool, RTLNodePtr);
extern ldrNodePtr lde_extract_dc(const symbolArr&, int, Regexp*, bool, RTLNodePtr);

#endif

/*
   START-LOG-------------------------------------------

   $Log: lde_erd.h  $
   Revision 1.2 1995/11/07 20:14:20EST wmm 
   Bug track: 10719, 11160, 11126, 11180
   Also fix unscriptable bugs 10135, 10667, 10899, 10881.
 * Revision 1.2.1.4  1993/01/22  22:46:39  wmm
 * Support XREF-based ERDs and DCs.
 *
 * Revision 1.2.1.3  1992/12/17  21:29:29  wmm
 * Add lde_extract_dc for Data Charts.
 *
 * Revision 1.2.1.2  1992/10/09  19:38:12  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
