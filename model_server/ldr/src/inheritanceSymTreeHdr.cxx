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
// /aset/ldr/src/inheritanceSymTreeHdr.C
//------------------------------------------
// description:
// 
//------------------------------------------
// History:
//
// Thu Apr 20 09:42:01 1995:	Created by William M. Miller
//
//------------------------------------------
#include "genError.h"


#include <inheritanceSymTreeHdr.h>

#ifndef __ldrNode_h_
#include <ldrNode.h>
#endif

#ifndef _ldrBrowserHierarchy_h_
#include <ldrBrowserHierarchy.h>
#endif

#ifndef _objOper_h
#include <objOper.h>
#endif

#include <ddict.h>
#ifndef _dd_or_xref_node_h
#include <dd_or_xref_node.h>
#endif



//****************************************
// Implementations for class inheritanceSymTreeHdr
//****************************************

init_relational(inheritanceSymTreeHdr,symbolTreeHeader);


//========================================
// inheritanceSymTreeHdr::inheritanceSymTreeHdr
//----------------------------------------
// Description:
//
// No local processing, just passes name argument to base class.
//----------------------------------------
// History:
// 
// Thu Apr 20 09:54:25 1995:	Created by William M. Miller
//========================================

inheritanceSymTreeHdr::inheritanceSymTreeHdr(char* name): symbolTreeHeader(name) { }


//========================================
// inheritanceSymTreeHdr::xref_notify
//----------------------------------------
// Description:
//
// Checks to see if the view's root class has been deleted and deletes the view
// if so; otherwise, just passes the arguments along to symbolTreeHeader for
// processing.
//----------------------------------------
// History:
// 
// Thu Apr 20 09:55:52 1995:	Created by William M. Miller
//========================================

void inheritanceSymTreeHdr::xref_notify(const symbolArr& modified,
					const symbolArr& added,
					const symbolArr& deleted) {
   Initialize(inheritanceSymTreeHdr::xref_notify);

   Obj* ldrhs = app_get_ldrs(this);	// Can only be 1, since this is created
	 				// as part of ldr extraction
   Obj*	ldrh;
   ForEach(ldrh, *ldrhs) {
      ldrBrowserHierarchyPtr inh_ldr = checked_cast(ldrBrowserHierarchy, ldrh);
      ldrTreePtr ldr_root = checked_cast(ldrTree, inh_ldr->get_root());
      appTreePtr app_root = (ldr_root) ? ldr_root->get_appTree() : NULL;
      if (app_root && is_dd_or_xref_node(app_root)) {
	 symbolPtr this_sym = dd_or_xref_nodePtr(app_root)->get_xrefSymbol();
	 symbolPtr del_sym;
	 ForEachS(del_sym, deleted) {
	    if (this_sym == del_sym) {
	       obj_delete(this);
	       return;
	    }
	 }
      }
   }
   symbolTreeHeader::xref_notify(modified, added, deleted);
}
