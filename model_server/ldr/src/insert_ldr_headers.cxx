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
// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <msg.h>
#include <ddict.h>
#include <dd_or_xref_node.h>
#include <messages.h>
#include <ldrNode.h>
#include <transaction.h>

void insert_ldr_headers(dd_or_xref_node *dd) {
   Initialize(insert_ldr_headers);
   Obj* ldrnodes = apptree_get_ldrtrees(dd);
   objSet ldrhdrs;
   Obj* ldrnode;
   {
      ForEach(ldrnode, *ldrnodes) {
	 Obj* h = checked_cast(ldrTree, ldrnode)->get_header();
	 if (h) {
	    ldrhdrs.insert(h);
	 }
      }
   }
   start_transaction() {
      Obj* ldrhdr;
      ForEach(ldrhdr, ldrhdrs) {
	 appPtr ah = checked_cast(ldr, ldrhdr)->get_appHeader();
	 if (ah) {
	    appTreePtr at = checked_cast(appTree, ah->get_root());
	    if (at)
	       obj_insert(ah, REPLACE, at, at, NULL);
	 }
      }
   } end_transaction();
}

