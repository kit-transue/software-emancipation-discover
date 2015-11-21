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
// Methods for ldrERDHierarchy -- ldr header for hierarchy
// of Entity Relation Diagrams.


#include "machdep.h"
#include <msg.h>
#include <genError.h>
#include <objOperate.h>
#include <ldrError.h>
#include <ldrNode.h>
#include <ldrERDHierarchy.h>
#include <lde_erd.h>
#include <ddHeader.h>
#include <symbolTreeHeader.h>
#include <dd_or_xref_node.h>
#include <RTLdialog.h>
#include <gtPushButton.h>
#include <oodt_ui_decls.h>
#include <messages.h>
#include <proj.h>

//#define TXT(a) a

// this should be a method of ldrOODT
static void
insert_pending(ldrOODTPtr ldrp, const symbolArr &contents)
{
	Initialize(insert_pending);
	int index, size = contents.size();
	for(index = 0; index < size; ++index) {
		fsymbolPtr  xsym = contents[index].get_xrefSymbol();
		if(xsym.xrisnotnull() && xsym.is_loaded() &&
		   xsym->get_kind() != DD_SUBSYSTEM) {
			ddElement *def_dd = xsym->get_def_dd();
			if(def_dd) {
		// add_pending should not be called here since
		// it also plays with the update buttons which will
		// be done by refresh_pending below. keep for now...
				ldrp->add_pending(checked_cast(smtHeader, def_dd->get_main_header()));
			}
		}
	}
	ldrp->refresh_pending();
}


init_relational(ldrERDHierarchy,ldrOODT);

//------------------------------------------
// ldrERDHierarchy::ldrERDHierarchy(const symbolArr&)
//------------------------------------------

ldrERDHierarchy::ldrERDHierarchy(const symbolArr& contents):
      builtin_relation_filter(points_to_rel) {
   Initialize(ldrERDHierarchy::ldrERDHierarchy);

   symbolTreeHeaderPtr sth = db_new(symbolTreeHeader, (""));
   RTLNodePtr sym_root = checked_cast(RTLNode, sth->get_root());
   ldrNodePtr root = lde_extract_erd( contents, builtin_relation_filter, relation_filter, inh_rels, sym_root);
   ldr_put_app(this, sth);
   put_root(root);
   set_type(Rep_ERD); 

	insert_pending(this, contents);
}


//------------------------------------------
// ldrERDHierarchy::insert_obj(...)
//------------------------------------------

void ldrERDHierarchy::insert_obj(objInserter *oi, objInserter *ni) {
   Initialize(ldrERDHierarchy::insert_obj);

   refresh_pending();
   rebuild_root();
   if (oi->src_obj)
      ni->src_obj = find_ldr(oi->src_obj);
   if (oi->targ_obj)
      ni->targ_obj = find_ldr(oi->targ_obj);
}

//------------------------------------------
// ldrERDHierarchy::remove_obj(...)
//------------------------------------------

void ldrERDHierarchy::remove_obj(objRemover *ro, objRemover *nr) {
   Initialize(ldrERDHierarchy::insert_obj);

   ldrNodePtr old_root = checked_cast(ldrNode, get_root());
   symbolArr classes;
   for (ldrNodePtr p = checked_cast(ldrNode, old_root->get_first()); p;
	p = checked_cast(ldrNode, p->get_next())) {
      if (is_ldrSymbolNode(p)) {
	 ldrSymbolType type = ldrSymbolNodePtr(p)->get_symbol_type();
	 if (type == ldrERD_CLASS || type == ldrCURRENT_SUBSYSTEM ||
	       type == ldrXREF_ERD_CLASS) {
	    appTreePtr node = p->get_appTree();
	    if (node != ro->src_obj) {
	       if (is_dd_or_xref_node(node)) {
		  classes.insert_last(dd_or_xref_nodePtr(node)->
				      get_xrefSymbol());
	       }
	       else classes.insert_last(node);
	    }
	 }
      }
   }

   symbolTreeHeaderPtr old_sth = checked_cast(symbolTreeHeader,
         get_appHeader());
   symbolTreeHeaderPtr sth = db_new(symbolTreeHeader, (""));
   RTLNodePtr sym_root = checked_cast(RTLNode, sth->get_root());
   nr->src_obj = find_ldr(ro->src_obj);
   put_root(lde_extract_erd( classes, builtin_relation_filter, relation_filter, inh_rels, sym_root));
   obj_delete(old_root);
   ldr_put_app(this, sth);
   obj_delete(old_sth);
}

//------------------------------------------
// ldrERDHierarchy::remove_class(symbolPtr)
//
// Take a particular class out of the ERD (this does *not* delete it from
// the world)
//------------------------------------------

void ldrERDHierarchy::remove_class(symbolPtr class_sym) {
   Initialize(ldrERDHierarchy::remove_class);

   symbolArr classes;
   int found_something = 0;
   for (ldrNodePtr p = checked_cast(ldrNode, get_root()->get_first()); p;
	p = checked_cast(ldrNode, p->get_next())) {
      if (is_ldrSymbolNode(p)) {
	 ldrSymbolType type = ldrSymbolNodePtr(p)->get_symbol_type();
	 if (type == ldrERD_CLASS || type == ldrXREF_ERD_CLASS) {
	    dd_or_xref_nodePtr node = checked_cast(dd_or_xref_node,
                  p->get_appTree());
	    fsymbolPtr node_xref = node->get_xrefSymbol();
	    if (class_sym.get_xrefSymbol() != node_xref)
	      classes.insert_last(node_xref);
	    else found_something = 1;
	 }
      }
   }
   if (found_something) {
      ldrNodePtr old_root = checked_cast(ldrNode, get_root());
      symbolTreeHeaderPtr old_sth = checked_cast(symbolTreeHeader,
            get_appHeader());
      symbolTreeHeaderPtr sth = db_new(symbolTreeHeader, (""));
      RTLNodePtr sym_root = checked_cast(RTLNode, sth->get_root());
      put_root(lde_extract_erd( classes, builtin_relation_filter, relation_filter, inh_rels, sym_root));
      obj_delete(old_root);
      ldr_put_app(this, sth);
      obj_delete(old_sth);
      obj_insert(sth, REPLACE, sym_root, sym_root, NULL);	// propagate
   }
   else msg("ERROR: ldrDCHierarchy::remove -- couldn't find item to remove") << eom;
}	   

//------------------------------------------
// ldrERDHiearchy::add(symbolPtr)
//
// Add an existing class to the ERD.
//------------------------------------------

void ldrERDHierarchy::add(symbolPtr class_sym) {
   Initialize(ldrERDHierarchy::add);

   symbolArr classes;
   for (ldrNodePtr p = checked_cast(ldrNode, get_root()->get_first()); p;
	p = checked_cast(ldrNode, p->get_next())) {
      if (is_ldrSymbolNode(p)) {
	 ldrSymbolType type = ldrSymbolNodePtr(p)->get_symbol_type();
	 if (type == ldrERD_CLASS || type == ldrCURRENT_SUBSYSTEM ||
	       type == ldrXREF_ERD_CLASS) {
	    appTreePtr node = p->get_appTree();
	    if (is_dd_or_xref_node(node)) {
	       classes.insert_last(dd_or_xref_nodePtr(node)->
				   get_xrefSymbol());
	    }
	    else classes.insert_last(node);
	 }
      }
   }
   classes.insert_last(class_sym);

   ldrNodePtr old_root = checked_cast(ldrNode, get_root());
   symbolTreeHeaderPtr old_sth = checked_cast(symbolTreeHeader,
         get_appHeader());
   symbolTreeHeaderPtr sth = db_new(symbolTreeHeader, (""));
   RTLNodePtr sym_root = checked_cast(RTLNode, sth->get_root());
   put_root(lde_extract_erd( classes, builtin_relation_filter, relation_filter, inh_rels, sym_root));
   obj_delete(old_root);
   ldr_put_app(this, sth);
   obj_delete(old_sth);
   obj_insert(sth, REPLACE, sym_root, sym_root, NULL);	// propagate
}

//------------------------------------------
// ldrERDHierarchy::filter_builtin_relations(int)
//------------------------------------------

void ldrERDHierarchy::filter_builtin_relations(int filter) {
   Initialize(ldrERDHierarchy::filter_builtin_relations);

   builtin_relation_filter = filter;

   rebuild_root();
}

//------------------------------------------
// ldrERDHierarchy::rebuild_root()
//
// Reextract ERD, using current objects
//------------------------------------------

void ldrERDHierarchy::rebuild_root() {
   Initialize(ldrERDHierarchy::rebuild_root);

// global control of view refreshing during propagation
   if (doNotRefreshPropagated(this))
        return;
    
   symbolArr classes;
   ldrNodePtr old_root = checked_cast(ldrNode, get_root());
   symbolTreeHeaderPtr old_sth = checked_cast(symbolTreeHeader,
         get_appHeader());
   projNodePtr home = projNode::get_home_proj();
   RTLNodePtr rtln = checked_cast(RTLNode, old_sth->get_root());   
   symbolArr & classes_arr = rtln->rtl_contents();
   symbolPtr csym;
   ForEachS(csym, classes_arr) {
       if(csym.is_xrefSymbol()){  //crash protection for following statement. 7/11/94.
          ddKind kind = csym->get_kind();
          if (kind==DD_CLASS || kind==DD_SUBSYSTEM) {
	     symbolPtr home_sym = home->lookup_xrefSymbol_in_cur_proj(kind,
		     csym.get_name());
	     if (home_sym.xrisnotnull() && home_sym.get_has_def_file() &&
		 home_sym->get_def_file().get_has_def_file()) {
		classes.insert_last(home_sym);
	     }
	     else classes.insert_last(csym);
	  }
       }
   }

   classes.usort();

   symbolTreeHeaderPtr sth = db_new(symbolTreeHeader, (""));

   RTLNodePtr sym_root = checked_cast(RTLNode, sth->get_root());
   put_root(lde_extract_erd( classes, builtin_relation_filter, relation_filter, inh_rels, sym_root));
   obj_delete(old_root);
   ldr_put_app(this, sth);
   obj_delete(old_sth);
}

//------------------------------------------
// ldrERDHierarchy::find_ldr(objTreePtr)
//------------------------------------------

objTreePtr ldrERDHierarchy::find_ldr(objTreePtr source) 
{
    Initialize(ldrERDHierarchy::find_ldr);
    ldrTreePtr found_ldr = NULL;

    ldrTreePtr root = checked_cast(ldrTree, get_root());
    Obj * ldr_set = apptree_get_ldrtrees(checked_cast(appTree,source));

    Obj* lnode;
    ForEach(lnode, *ldr_set) {
       if (checked_cast(ldrTree,lnode)->get_header() == this) {
	  found_ldr = checked_cast(ldrTree, lnode);
	  break;
       }
    }

    return found_ldr;
}

//------------------------------------------
// ldrERDHierarchy::do_refresh
//------------------------------------------

void ldrERDHierarchy::do_refresh(const symbolArr& before, const symbolArr& after) {
   Initialize(ldrERDHierarchy::do_refresh);

   symbolArr classes;
   bool swapped_something = false;
   for (ldrNodePtr p = checked_cast(ldrNode, get_root()->get_first()); p;
	p = checked_cast(ldrNode, p->get_next())) {
      if (is_ldrSymbolNode(p)) {
	 ldrSymbolType type = ldrSymbolNodePtr(p)->get_symbol_type();
	 if (type == ldrERD_CLASS || type == ldrXREF_ERD_CLASS) {
	    dd_or_xref_nodePtr node = checked_cast(dd_or_xref_node,
                  p->get_appTree());
	    fsymbolPtr node_xref = node->get_xrefSymbol();
	    bool swapped_sym = false;
	    for (size_t i = 0; i < before.size(); i++) {
	       if (node_xref.sym_compare(before[i]) == 0) {
		  swapped_something = true;
		  swapped_sym = true;
		  classes.insert_last(after[i]);
		  break;
	       }
	       if (!swapped_sym) {
		  classes.insert_last(node_xref);
	       }
	    }
	 }
	 else if (type == ldrCURRENT_SUBSYSTEM) {
	    classes.insert_last(p->get_appTree());
	 }
      }
   }
   if (swapped_something) {
      ldrNodePtr old_root = checked_cast(ldrNode, get_root());
      symbolTreeHeaderPtr old_sth = checked_cast(symbolTreeHeader,
            get_appHeader());
      symbolTreeHeaderPtr sth = db_new(symbolTreeHeader, (""));
      RTLNodePtr sym_root = checked_cast(RTLNode, sth->get_root());
      put_root(lde_extract_erd( classes, builtin_relation_filter, relation_filter, inh_rels, sym_root));
      obj_delete(old_root);
      ldr_put_app(this, sth);
      obj_delete(old_sth);
      obj_insert(sth, REPLACE, sym_root, sym_root, NULL);	// propagate
   }
}

/*
   START-LOG-------------------------------------------

   $Log: ldrERDHierarchy.cxx  $
   Revision 1.19 2002/01/23 09:56:19EST ktrans 
   Merge from branch: mainly dormant code removal
Revision 1.2.1.22  1994/07/12  15:25:40  bhowmik
Bug track: 7681
Preventing crash in rebuild root by checking for xref_sym.

Revision 1.2.1.21  1994/01/24  19:32:04  davea
bug 6065
in rebuilt_root(), only copy the classes and subsystems,
not the members.  The latter will be reconstructed in at the
end of the function.

Revision 1.2.1.20  1994/01/17  16:30:43  trung
minor fixes in prop

Revision 1.2.1.19  1994/01/04  20:52:31  wmm
Bug track: 5746
Fix bug 5746.

Revision 1.2.1.18  1993/12/23  21:45:03  andrea
Bug track: 5578
I turned off all relations except points_to_rel

Revision 1.2.1.17  1993/12/21  22:45:34  wmm
Bug track: 5672
Fix bug 5672 by: 1) collecting all propagations of modified files in an RTL in Hierarchical and
doing the propagation at end_transaction time; 2) removing the relation between the ddHeader
and OODT view symbolTrees.

Revision 1.2.1.16  1993/10/28  16:24:09  andrea
Bug track: 5001
I set the type in the constructor to Rep_ERD

Revision 1.2.1.15  1993/09/22  01:22:55  pero
Bug track: 4776
modify to update view at the end of propagation (class def of ldrOODT and
viewGraHeader)

Revision 1.2.1.14  1993/09/20  02:26:51  pero
Bug track: 4776
only refresh view after propagation is done (rebuild_root)

Revision 1.2.1.13  1993/08/30  21:46:28  wmm
Fix bug 4621.

Revision 1.2.1.12  1993/08/05  23:20:42  wmm
Fix bug 4185 (allow deferred parsing from OODT views).

Revision 1.2.1.11  1993/06/18  21:20:23  wmm
Hook up "help" buttons, change some terminology to be consistent with
C++ and ParaSET usage.

Revision 1.2.1.10  1993/03/16  13:56:08  davea
change xrefSymbol* to fsymbolPtr

Revision 1.2.1.9  1993/03/15  17:45:23  wmm
Fix bug 2845 (adding and removing classes to ERDs stopped updating the
view, so I put in an explicit obj_insert(REPLACE) to do the job).

Revision 1.2.1.8  1993/02/06  22:44:03  wmm
Propagate notification of import (default propagation
occurs too early, before the SMT is parsed).

Revision 1.2.1.7  1993/02/05  16:27:34  wmm
Support "import and show details" functionality.

Revision 1.2.1.6  1993/02/01  18:10:05  wmm
Fix bug 2347.

Revision 1.2.1.5  1993/01/22  22:48:28  wmm
Support XREF-based ERDs and DCs.

Revision 1.2.1.4  1992/12/17  21:30:57  wmm
Fix erroneous cast (ldrERDConnectionNodes are not ldrSymbolNodes, but the
code treated them as if they were).

Revision 1.2.1.3  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:43:48  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/

