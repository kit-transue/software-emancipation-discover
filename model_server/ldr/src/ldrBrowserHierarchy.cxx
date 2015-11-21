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
// Methods for ldrBrowserHierarchy -- ldr header for hierarchy
// diagrams of the OODT class browser.


#include "machdep.h"
#include "genError.h"
#include "objOperate.h"
#include "ldrError.h"
#include "ldrNode.h"
#include "ldrBrowserHierarchy.h"
#include "lde-hierarchy.h"
#include "lde-browser-hierarchy.h"
#include "ddict.h"
#include <inheritanceSymTreeHdr.h>
#include <dd_or_xref_node.h>
#include <RTLdialog.h>
#include <gtPushButton.h>

#ifndef _proj_h
#include <proj.h>
#endif

#define TXT(a) a

init_relational(ldrBrowserHierarchy,ldrOODT);

ldrBrowserHierarchy::ldrBrowserHierarchy(symbolPtr tr) {
   Initialize(ldrBrowserHierarchy::ldrBrowserHierarchy);

   inheritanceSymTreeHdrPtr sth = db_new(inheritanceSymTreeHdr, (""));
   RTLNodePtr sym_root = checked_cast(RTLNode, sth->get_root());
   ldrNodePtr root = lde_extract_browser_hierarchy(tr, sym_root);
   ldr_put_app(this, sth);
   put_root(root);
}

void ldrBrowserHierarchy::reroot(symbolPtr tr) {
   Initialize(ldrBrowserHierarchy::reroot);

   static inheritanceSymTreeHdrPtr dummy_header = db_new(inheritanceSymTreeHdr, (""));
   start_transaction() {
      ldrNodePtr old_root = checked_cast(ldrNode, get_root());
      inheritanceSymTreeHdrPtr sth = checked_cast(inheritanceSymTreeHdr,
            get_appHeader());
      RTLNodePtr sym_root = checked_cast(RTLNode, sth->get_root());
      ldr_put_app(this, dummy_header);	// prevent propagation of clear()
      sym_root->clear();
      objTreePtr p;
      while (p = sym_root->get_first()) {
         p->remove_from_tree();
         obj_delete(p);
      }
      ldr_put_app(this, sth);
      put_root(lde_extract_browser_hierarchy(tr, sym_root));
      obj_delete(old_root);
   } end_transaction();
}

void ldrBrowserHierarchy::rebuild_root() {
   Initialize(ldrBrowserHierarchy::rebuild_root);
   
// global control of view refreshing during propagation
   if (doNotRefreshPropagated(this))
        return;
 
   ldrTreePtr ldr_root = checked_cast(ldrTree, get_root());
   appTreePtr app_root = (ldr_root) ? ldr_root->get_appTree() : NULL;
   if (app_root && is_dd_or_xref_node(app_root)) {
      // Refresh to home proj symbol after check-out.  Must be careful
      // to avoid "reference" home project symbols that result from
      // oodt_relations or groups.  These are categorized either by not
      // having a def_file or by the def_file not having a def_file.
      projNodePtr home = projNode::get_home_proj();
      symbolPtr org_sym = dd_or_xref_nodePtr(app_root)->get_xrefSymbol();
      symbolPtr home_sym = home->lookup_xrefSymbol_in_cur_proj(DD_CLASS,
	      org_sym.get_name());
      if (home_sym.xrisnotnull() && home_sym.get_has_def_file() &&
	  home_sym->get_def_file().get_has_def_file()) {
	 reroot(home_sym);
      }
      else reroot(org_sym);
   }
}

void ldrBrowserHierarchy::insert_obj(objInserter *, objInserter *ni) {
   Initialize(ldrBrowserHierarchy::insert_obj);

   refresh_pending();
   rebuild_root();
   ldrNodePtr new_root = checked_cast(ldrNode, get_root());
   ni->type = REPLACE;
   ni->src_obj = new_root;
   ni->targ_obj = new_root;
}

void ldrBrowserHierarchy::remove_obj(objRemover*, objRemover*) {
   Initialize(ldrBrowserHierarchy::insert_obj);

   rebuild_root();
}

objTreePtr ldrBrowserHierarchy::find_ldr(objTreePtr source) 
{
    Initialize(ldrBrowserHierarchy::find_ldr);
    ldrTreePtr found_ldr = NULL;

    ldrTreePtr root = checked_cast(ldrTree, get_root());
    Obj * ldr_set = apptree_get_ldrtrees(checked_cast(appTree, source));
    Obj* lnode;
    ForEach(lnode, *ldr_set) {
       if (lnode != root && checked_cast(ldrTree, lnode)->get_header() ==
             this) {
	  found_ldr = checked_cast(ldrTree, lnode);
	  break;
       }
    }
    return found_ldr;
}


//------------------------------------------
// ldrBrowserHierarchy::add_inherited_members(...)
//------------------------------------------

void ldrBrowserHierarchy::add_inherited_members(ldrClassNodePtr the_class,
						objArr& added_members) {
   Initialize(ldrBrowserHierarchy::add_inherited_members);

   lde_browser_add_inherited_members(the_class, added_members);
}

//------------------------------------------
// ldrBrowserHierarchy::add_relations
//------------------------------------------

void ldrBrowserHierarchy::add_relations (ldrClassNodePtr cl, objArr& add)
{
    Initialize (ldrBrowserHierarchy::add_relations);

    lde_browser_add_relations (cl, add);
}

//------------------------------------------
// ldrBrowserHierarchy::do_refresh
//------------------------------------------

void ldrBrowserHierarchy::do_refresh(const symbolArr& before, const symbolArr& after) {
   Initialize(ldrBrowserHierarchy::do_refresh);

   ldrTreePtr ldr_root = checked_cast(ldrTree, get_root());
   appTreePtr app_root = (ldr_root) ? ldr_root->get_appTree() : NULL;
   if (app_root && is_dd_or_xref_node(app_root)) {
      symbolPtr org_sym = dd_or_xref_nodePtr(app_root)->get_xrefSymbol();
      for (size_t i = 0; i < before.size(); i++) {
	 if (org_sym.sym_compare(before[i]) == 0) {
	    reroot(after[i]);
	    return;
	 }
      }
      reroot(org_sym);
   }
}

/*
   START-LOG-------------------------------------------

   $Log: ldrBrowserHierarchy.cxx  $
   Revision 1.10 2002/01/23 09:56:16EST ktrans 
   Merge from branch: mainly dormant code removal
 * Revision 1.1  1994/09/21  18:44:09  jerry
 * Initial revision
 *
Revision 1.2.1.19  1993/12/21  22:44:24  wmm
Bug track: 5672
Fix bug 5672 by: 1) collecting all propagations of modified files in an RTL in Hierarchical and
doing the propagation at end_transaction time; 2) removing the relation between the ddHeader
and OODT view symbolTrees.

Revision 1.2.1.18  1993/09/22  01:22:01  pero
Bug track: 4776
modify to update view at the end of propagation (class def of ldrOODT and
viewGraHeader)

Revision 1.2.1.17  1993/09/20  02:26:01  pero
Bug track: 4776
only refresh view after propagation is done (rebuild_root)

Revision 1.2.1.16  1993/08/30  21:46:03  wmm
Fix bug 4621.

Revision 1.2.1.15  1993/08/05  23:20:37  wmm
Fix bug 4185 (allow deferred parsing from OODT views).

Revision 1.2.1.14  1993/07/02  13:56:38  wmm
Fix bug 3810 by only extracting superclass members if requested by the
user, and then only for the selected classes.

Revision 1.2.1.13  1993/06/18  21:20:01  wmm
Hook up "help" buttons, change some terminology to be consistent with
C++ and ParaSET usage.

Revision 1.2.1.12  1993/04/16  15:34:10  wmm
Fix bug 3363.

Revision 1.2.1.11  1993/04/14  10:58:42  wmm
Performance improvement: ensure that reroot is called only once
per propagation by reusing existing symbolTreeHeader.

Revision 1.2.1.10  1993/03/15  21:47:45  davea
change xrefSymbol* to  fsymbolPtr

Revision 1.2.1.9  1993/02/06  22:44:03  wmm
Propagate notification of import (default propagation
occurs too early, before the SMT is parsed).

Revision 1.2.1.8  1993/02/05  16:27:34  wmm
Support "import and show details" functionality.

Revision 1.2.1.7  1993/02/01  18:10:05  wmm
Fix bug 2347.

Revision 1.2.1.6  1993/01/24  01:15:34  wmm
Support XREF-based class browser view.

Revision 1.2.1.5  1993/01/10  02:51:00  wmm
Move setting app_of_ldr relation until after extraction is done,
so automatic import during extraction won't try to propagate
to this header.

Revision 1.2.1.4  1992/12/11  16:21:38  aharlap
changed places with direct access to xref

Revision 1.2.1.3  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:43:47  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/



