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
#ifndef _lde_browser_hierarch_h_
#define _lde_browser_hierarch_h_

#include "genError.h"
#include "ldrNode.h"
#include "ldrApplTypes.h"
#include <symbolPtr.h>
#include <objArr.h>

RelClass(RTLNode);

extern ldrNodePtr lde_extract_browser_hierarchy(symbolPtr, RTLNodePtr);
extern void lde_browser_add_inherited_members(ldrClassNodePtr, objArr&);
extern void lde_browser_add_relations (ldrClassNodePtr, objArr&);

#endif

/*
   START-LOG-------------------------------------------

   $Log: lde-browser-hierarchy.h  $
   Revision 1.2 1994/09/21 17:15:12EDT jerry 
   Bug track: None
   Disabled default relation cartouches in the inheritance diagram.  This is meant
   to decrease the initial start-up time for the diagram.
 * Revision 1.1  1994/09/21  18:43:16  jerry
 * Initial revision
 *
 * Revision 1.2.1.4  1993/07/02  13:55:20  wmm
 * Fix bug 3810 by only extracting superclass members if requested by the
 * user, and then only for the selected classes.
 *
 * Revision 1.2.1.3  1993/01/24  01:12:45  wmm
 * Support XREF-based class browser view.
 *
 * Revision 1.2.1.2  1992/10/09  19:38:06  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
