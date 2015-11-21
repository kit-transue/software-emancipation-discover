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
// dd_or_xref_node.h.C
//------------------------------------------
// synopsis:
// 
// Implementation of dd_or_xref_node; see the .h file for details.
//------------------------------------------

// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <msg.h>
#include <ddict.h>
#include <dd_or_xref_node.h>
#include <messages.h>
//#include <ldrNode.h>
#include <transaction.h>


void db_set_incremental ();
// PRE-PROCESSOR DEFINITIONS


init_relational(dd_or_xref_node, appTree);

init_relation(elem_of_dd_or_xref_node,1,NULL,dd_or_xref_node_of_elem,MANY,NULL);

void dd_or_xref_node::print( ostream &str, int /*level*/)  const
{
  symbolPtr sym = get_xrefSymbol();

  str << ddKind_name(sym->get_kind()) << ' ' << sym.get_name();
}

//------------------------------------------
// dd_or_xref_node::send_string(ostream&) const
//------------------------------------------

void dd_or_xref_node::send_string(ostream& os) const {
   Initialize(dd_or_xref_node::send_string);

   ddElementPtr elem = dd_or_xref_node_get_elem(this);
   if (elem)
      elem->send_string(os);
   else if (xref_ptr.xrisnotnull())
      os << xref_ptr.get_name();
   else msg("ERROR: Internal error: unattached dd_or_xref_node::send_string") << eom;
}

//------------------------------------------
// dd_or_xref_node::get_name() const
//------------------------------------------

char const *dd_or_xref_node::get_name() const {
   Initialize(dd_or_xref_node::get_name);

   ddElementPtr elem = dd_or_xref_node_get_elem(this);
   if (elem)
      return elem->get_ddname();
   else if (xref_ptr.xrisnotnull())
      return xref_ptr.get_name();
   else msg("ERROR: Internal error: unattached dd_or_xref_node::get_name") << eom;
   return NULL;
}

//------------------------------------------
// dd_or_xref_node::dd_or_xref_node(ddElementPtr)
//------------------------------------------

dd_or_xref_node::dd_or_xref_node(ddElementPtr elem):
      xref_ptr(elem->get_xrefSymbol()) {
   Initialize(dd_or_xref_node::dd_or_xref_node);

//  Following is not needed, because initializer already made sure the
//    contents of the symbolPtr xref_ptr is an xrefSymbol
//   if (xref_ptr.xrisnotnull())
//      xref_ptr = xref_ptr->get_real_sym();
   elem_put_dd_or_xref_node(elem, this);
}

//------------------------------------------
// dd_or_xref_node::dd_or_xref_node(symbolPtr)
//------------------------------------------

dd_or_xref_node::dd_or_xref_node(symbolPtr sym):
      xref_ptr(sym) {
   Initialize(dd_or_xref_node);

   if (xref_ptr.xrisnotnull()) {
      xref_ptr = xref_ptr.get_xrefSymbol();
      if (xref_ptr.is_loaded()) {
	 ddElementPtr elem = xref_ptr->get_def_dd();
	 if (elem) {
	    elem_put_dd_or_xref_node(elem, this);
	    xref_ptr = elem->get_xrefSymbol();
	 }
      }
   }
   else msg("ERROR: Internal error: unattached dd_or_xref_node::ctor") << eom;
}

//------------------------------------------
// dd_or_xref_node::dd_or_xref_node(const dd_or_xref_node&)
//------------------------------------------

dd_or_xref_node::dd_or_xref_node(const dd_or_xref_node& other):
      appTree(other), xref_ptr(other.xref_ptr) {
   Initialize(dd_or_xref_node::dd_or_xref_node);

   ddElementPtr elem = dd_or_xref_node_get_elem(&other);
   if (!elem && xref_ptr.xrisnotnull() && xref_ptr.is_loaded())
      elem = xref_ptr->get_def_dd();
   if (elem) {
      elem_put_dd_or_xref_node(elem, this);
      xref_ptr = elem->get_xrefSymbol();
   }
}

void insert_ldr_headers (dd_or_xref_node *);

//------------------------------------------
// dd_or_xref_node::get_dd()
//------------------------------------------

ddElementPtr dd_or_xref_node::get_dd() {
   Initialize(dd_or_xref_node::get_dd);

   ddElementPtr elem = dd_or_xref_node_get_elem(this);
   boolean	loaded_here;

   if (!elem && xref_ptr.xrisnotnull()) {
      start_transaction() {
	 loaded_here = !xref_ptr.is_loaded();
	 elem = xref_ptr->get_def_dd();
	 if (elem && elem->get_id() > 0) {
            elem_put_dd_or_xref_node(elem, this);
	    xref_ptr = elem->get_xrefSymbol();
	 }
      } end_transaction();
      if (loaded_here && (elem && elem->get_id() > 0)) {
         insert_ldr_headers(this);
      }
   }
   return elem;
}

//------------------------------------------
// dd_or_xref_node::get_dd_if_loaded()
//------------------------------------------

ddElementPtr dd_or_xref_node::get_dd_if_loaded() {
   Initialize(dd_or_xref_node::get_dd_if_loaded);

   ddElementPtr elem = dd_or_xref_node_get_elem(this);
   if (!elem && xref_ptr.xrisnotnull() && xref_ptr.is_loaded()) {
      elem = xref_ptr->get_def_dd();
      if (elem && elem->get_id() <= 0)
	 elem = NULL;
      if (elem) {
	 elem_put_dd_or_xref_node(elem, this);
	 xref_ptr = elem->get_xrefSymbol();
      }
   }
   return elem;
}

//------------------------------------------
// dd_or_xref_node::get_xrefSymbol() const
//------------------------------------------

fsymbolPtr dd_or_xref_node::get_xrefSymbol() const {
   Initialize(dd_or_xref_node::get_xrefSymbol);

   return xref_ptr;
}

int dd_or_xref_node::construct_symbolPtr(symbolPtr & sym) const
{
    sym = get_xrefSymbol();
    return 1;
}

/*
   START-LOG-------------------------------------------

   $Log: dd_or_xref_node.h.cxx  $
   Revision 1.14 2001/07/25 20:41:29EDT Sudha Kallem (sudha) 
   Changes to support model_server.
// Revision 1.26  1994/02/14  06:08:29  trung
// do make room for doing batch
//
// Revision 1.25  1994/02/01  16:13:38  trung
// Bug track: 6065
// change new_node in lde_erd
//
// Revision 1.24  1993/12/22  16:34:39  aharlap
// add db_set_incremental
//
// Revision 1.23  1993/09/11  22:26:22  trung
// #4718
//
// Revision 1.22  1993/08/26  02:38:21  aharlap
// bug # 4561
//
// Revision 1.21  1993/07/26  23:04:05  wmm
// Fix bug 4130.
//
// Revision 1.20  1993/07/09  18:46:07  wmm
// Partial fix for bug 3939.
//
// Revision 1.19  1993/06/25  13:55:41  bakshi
// remove redundant constructor for c++3.0.1 port
//
// Revision 1.18  1993/05/17  17:32:17  wmm
// Performance tuning, etc., for subsystem extraction.
//
// Revision 1.17  1993/04/22  13:23:34  davea
// bug 3454 - added return stmt to get_name()
//
// Revision 1.16  1993/04/14  11:00:13  wmm
// Partial fix of bug 3108 (do not close yet): put propagation
// from get_dd() into transaction protection.
//
// Revision 1.15  1993/04/08  15:07:04  wmm
// Check for deleted ddElements (part of bug 3191).
//
// Revision 1.14  1993/03/30  22:21:00  davea
// added get_xrefSymbol() for bug 2927
//
// Revision 1.13  1993/03/29  19:50:56  trung
// rm call to get_real_sym
//
// Revision 1.12  1993/03/27  01:44:44  davea
// Changes caused by going from xrefSymbol* to symbolPtr
//
// Revision 1.10  1993/03/01  21:49:15  wmm
// Allow for cases in which xref_ptr may be NULL and don't call
// get_real_sym() for NULL pointers.
//
// Revision 1.9  1993/03/01  19:29:44  wmm
// Use xrefSymbol::get_real_sym() in initializing xref_ptr to
// cope with name changes.
//
// Revision 1.8  1993/02/12  19:42:22  wmm
// Fix bug in which ldr could be updated multiple times after
// a get_dd() operation, causing delays and crashes.
//
// Revision 1.7  1993/02/06  22:42:43  wmm
// Propagate notification of import to ldrs (default propagation
// occurs too early, before the SMT is parsed).
//
// Revision 1.6  1993/02/05  16:25:48  wmm
// Remove spurious "unattached node" message (get_dd() causes an
// import, and if the import fails, there will be no DD element).
//
// Revision 1.5  1993/02/05  16:21:59  wmm
// Protect against crash caused by premature deletion of node for
// which get_dd() is invoked (it can result in redoing the ldr,
// which deletes the corresponding appTree nodes).
//
// Revision 1.4  1993/02/03  23:44:05  wmm
// Load DD for symbols if definition file has .pset file.
//
// Revision 1.3  1993/01/29  20:38:32  wmm
// Fix bug 2344.
//
// Revision 1.2  1993/01/24  01:12:11  wmm
// Add get_name() member function.
//
// Revision 1.1  1993/01/22  22:45:22  wmm
// Initial revision
//
   END-LOG---------------------------------------------
*/
