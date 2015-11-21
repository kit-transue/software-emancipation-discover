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
#ifndef __lde_RTL_h_
#define __lde_RTL_h_

#include "ldrRTLNode.h"
#include "ldrRTLDisplayType.h"

extern ldrRTLNodePtr lde_extract_RTL_tree (RTLNodePtr, ldrRTLDisplayType);

extern ldrRTLNodePtr lde_create_RTLldrNode (symbolPtr&, ldrRTLDisplayType);

extern ldrRTLNodePtr lde_find_RTLldrNode (ldrRTLNodePtr first, symbolPtr);

extern ldrRTLNodePtr lde_insert_RTLldrNode (ldrRTLNodePtr first, ldrRTLNodePtr
				   ldr_node);

extern ldrRTLNodePtr lde_insert_obj_in_RTLldrNode (ldrRTLNodePtr
						   first_ldr, Obj *obj,
						   int ascending,
						   int graphlevel);

extern void lde_decorate (ldrNodePtr node);

extern void lde_detach (ldrNodePtr);

extern void lde_RTL_insert_first (ldrRTLNodePtr, ldrRTLNodePtr);
extern void lde_RTL_insert_after (ldrRTLNodePtr, ldrRTLNodePtr);
extern void lde_RTL_replace      (ldrRTLNodePtr, ldrRTLNodePtr);

#endif

/*
   START-LOG-------------------------------------------

   $Log: lde-RTL.h  $
   Revision 1.1 1993/07/29 10:38:33EDT builder 
   made from unix file
 * Revision 1.2.1.4  1992/12/15  19:29:30  aharlap
 * changed for new xref
 *
 * Revision 1.2.1.3  1992/12/09  20:21:05  aharlap
 * added interface for new xref
 *
 * Revision 1.2.1.2  92/10/09  19:37:43  jon
 * RCS History Marker Fixup
 * 


   END-LOG---------------------------------------------

*/
