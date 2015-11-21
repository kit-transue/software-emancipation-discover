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
#include "smt.h"

#ifndef _Relational_h
#include "relational.h"
#endif

#include "objRelation.h"
#include "ddict.h"
#include "ddKind.h"
#include "xref.h"
#include "astTree.h"
#include "ast_cplus_tree.h"
#include "RTL.h"
#include "RTL_externs.h"
#include "smode_categories.h"
#include "smt_categories.h"
#include <genString.h>


//-----------------------------------------------------------------------------
// 
// FILE CONTENTS:
//
// void smt_map_dd(...)              put a relation between SMT node and dd
// smt_get_AST_smode_categories(...) copy AST categories to SMT_decl node
// smt_get_decl_categories(...)      set decl node and right sibling categories
// dd_smt_find_decls_categories(...) set categories in SMT's decl nodes
// dd_smt_all_decls_categories(...)  nonrecursive launcher for this module
//
//-----------------------------------------------------------------------------

//------------------------------------------
// dd_map_smt(...)
//
// Looks up (or creates, if there isn't one) the dd entry for
// the string argument "name", and creates the specified
// relationship "rel" between the specified smtTree node "smt",
// and the dd entry.
// 
//------------------------------------------

void smt_map_dd(char *name, smtTree *smt, ddKind kind, RelType *rel)
{
	Initialize(smt_map_dd);

	smtHeader   *hdr = checked_cast(smtHeader,smt->get_header());
	ddRootPtr    dr  = dd_sh_get_dr(hdr);
	ddElementPtr dd  = dr->lookup(kind, name);

	if (!dd) {
		if (name && *name && rel) {
			// Create a dd element and establish relationship rel
			// between it and the smt node.
		        dd = dr->add_def( kind, strlen(name), name, 0 , name, 0, name);
			put_relation(rel, smt, dd);
			dd->set_datatype(smt->extype & 0xf);
		}
	} else {
		put_relation(rel,smt,dd);
	}
}
