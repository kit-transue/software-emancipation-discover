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
#include <ddict.h>
#include <dd_or_xref_node.h>
#include <ldrOODT.h>
#include <transaction.h>
static void collect_dd_or_xref_node_headers(objTree* t, objSet& os)
{
    Initialize(collect_dd_or_xref_node_headers);

    if (is_ddElement(t)) {    
	ddElement* dd = checked_cast(ddElement, t);
	Obj* nodes = get_relation(dd_or_xref_node_of_elem,dd);
	Obj* el;
	ForEach(el,*nodes) {
	    dd_or_xref_node* node = checked_cast(dd_or_xref_node,el);
	    app* h = checked_cast(app,node->get_header());
	    Obj* ldrs = checked_cast(ldr,get_relation(ldr_of_app,h));
	    Obj* ob;
	    ForEach(ob,*ldrs) {
		os.insert(ob);
	    }
	}

	for(objTree* ch = t->get_first(); ch; ch= ch->get_next())
	    collect_dd_or_xref_node_headers(ch, os);
    }
} 
	
// called from obj_unload()
// to find all erd, inheritance, etc. views, based on the given app
// and to delete these views to avoid "zomby" views

void dd_delete_dd_or_xref_node_views(Obj* obj)
{
    Initialize(delete_dd_or_xref_node_views);
    
    Relational* h = (Relational*)obj;

    if (!is_app(h))
	return;
    if (is_smtHeader(h)) {
	smtHeader * smth = (smtHeader*)h;
	if (!smth->foreign && smth->srcbuf)
	    smth->cut_ast();
    }
    ddRoot* dr = checked_cast (ddRoot,get_relation(ddRoot_of_smtHeader, h));
    if (dr == 0)
	return;

    start_transaction() {
	objSet wmm_headers;
	for(objTree* t = dr->get_first(); t; t = t->get_next())
	    collect_dd_or_xref_node_headers(t, wmm_headers);

	int do_it = 1;
	while (do_it) {
	    do_it = 0;
	    Obj* el;
	    ForEach(el,wmm_headers) {
		do_it = 1;
		wmm_headers.remove(el);
		if (!is_ldrOODT(RelationalPtr(el))) {
		    obj_unload(el);
		}
		break;
	    }
	}
    } end_transaction();
}

