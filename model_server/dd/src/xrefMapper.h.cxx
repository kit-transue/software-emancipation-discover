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
// xrefMapper.C
//------------------------------------------
// synopsis:
// 
// Implementation of classes from xrefMapper.h
//------------------------------------------

// INCLUDE FILES

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#include <cstring>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <xrefMapper.h>
#include <xref.h>

// CLASS DEFINITIONS

//------------------------------------------
// class xrefMapper_rep provides the representation of the shadow XrefTables.
// There is one xrefMapper_rep object for each distinct Xref* value in the
// registered symbolPtrs; they are placed into a singly-linked list

class xrefMapper_rep {
public:
   xrefMapper_rep();
   ~xrefMapper_rep();

   void add_item(void* item, const symbolPtr&);
   void* item(const symbolPtr&);

private:
   xrefMapper_rep* next;
   Xref* base;
   unsigned int max_offset;
   char* shadow;
   bool is_base_rep;
};

// FUNCTION DEFINITIONS

//------------------------------------------
// xrefMapper::xrefMapper()
//------------------------------------------

xrefMapper::xrefMapper(): rep_head(NULL) { }

//------------------------------------------
// xrefMapper::~xrefMapper()
//------------------------------------------

xrefMapper::~xrefMapper() {
   Initialize(xrefMapper::~xrefMapper);

   delete rep_head;
}

//------------------------------------------
// xrefMapper::add_item(void*, const symbolPtr&)
//------------------------------------------

void xrefMapper::add_item(void* item, const symbolPtr& sym) {
   Initialize(xrefMapper::add_item);

   if (!rep_head) {
      rep_head = new xrefMapper_rep;
   }

   rep_head->add_item(item, sym);
}

//------------------------------------------
// xrefMapper::item(const symbolPtr&) const
//------------------------------------------

void* xrefMapper::item(const symbolPtr& sym) const {
   Initialize(xrefMapper::item);

   if (!rep_head) {
      return NULL;
   }

   return rep_head->item(sym);
}

//------------------------------------------
// xrefMapper_rep::xrefMapper_rep()
//------------------------------------------

xrefMapper_rep::xrefMapper_rep(): next(NULL), base(NULL), max_offset(0), shadow(NULL),
      is_base_rep(true) { }

//------------------------------------------
// xrefMapper_rep::~xrefMapper_rep()
//------------------------------------------

xrefMapper_rep::~xrefMapper_rep() {
   Initialize(xrefMapper_rep::~xrefMapper_rep);

   delete next;
   delete shadow;
}

//------------------------------------------
// The following union type is used to allow for unaligned storage of
// item pointers in the shadow XrefTable.
//------------------------------------------

union alignment_xlat {
   void* void_ptr;
   char void_str[sizeof(void*)];
};

//------------------------------------------
// xrefMapper_rep::add_item(void*, const symbolPtr&)
//------------------------------------------

void xrefMapper_rep::add_item(void* item, const symbolPtr& sym) {
   Initialize(xrefMapper_rep::add_item);

   symbolPtr xr_sym = sym.get_xrefSymbol();
   Xref* sym_base = xr_sym.get_xref();
   unsigned int sym_offset = xr_sym.get_xrefsym_offset_real();
   if (sym_base) {
      if (sym_base == base || base == NULL) {
 	 if (base == NULL || sym_offset > max_offset - sizeof(void*)) {
	    unsigned int new_max = sym_base->max_offset();
	    if (sym_offset > new_max) {		// should not happen, but for safety:
	       new_max = sym_offset + sizeof(void*);
	    }
	    char* new_shadow = (char*) calloc(new_max, 1);
	    if (!new_shadow) {
	       return;
	    }
	    if (shadow) {
	       memcpy(new_shadow, shadow, max_offset);
	       free(shadow);
	    }
	    else base = sym_base;
	    shadow = new_shadow;
	    max_offset = new_max;
	 }
	 alignment_xlat u;
	 u.void_ptr = item;
	 memcpy(shadow + sym_offset, u.void_str, sizeof(void*));
      }
      else {
	 if (!next) {
	    next = new xrefMapper_rep;
	    next->is_base_rep = false;
	 }
	 next->add_item(item, xr_sym);
      }
   }
}

//------------------------------------------
// xrefMapper_rep::item(const symbolPtr&)
//------------------------------------------

void* xrefMapper_rep::item(const symbolPtr& sym) {
   Initialize(xrefMapper_rep::item);
   
   symbolPtr xr_sym = sym.get_xrefSymbol();
   Xref* sym_base = xr_sym.get_xref();
   unsigned int sym_offset = xr_sym.get_xrefsym_offset_real();
   void* assoc_item = NULL;
   static bool doing_relookup = false;
   if (sym_base) {
      if (base == sym_base) {
	 if (sym_offset <= max_offset - sizeof(void*)) {
	    alignment_xlat u;
	    memcpy(u.void_str, shadow + sym_offset, sizeof(void*));
	    assoc_item = u.void_ptr;
	 }
      }
      else if (next) {
	 assoc_item = next->item(xr_sym);
      }
      if (!assoc_item && is_base_rep && !doing_relookup) {
	 symbolPtr file_sym = sym->get_def_file();
	 if (file_sym.xrisnotnull()) {
	    symbolArr junk;
	    if (!file_sym.get_has_def_file()) {	// probably clone, look for def
	       if (file_sym->get_link(is_defined_in, junk)) {
		  file_sym = junk[0];
		  junk.removeAll();
	       }
	    }
	    Xref* file_base = file_sym.get_xref();
	    if (file_base && file_base != sym_base) {
	       symbolPtr other_xref_sym = file_base->find_symbol(junk,
		     sym->get_kind(), sym.get_name(),
		     file_sym.get_name());
	       doing_relookup = true;
	       assoc_item = item(other_xref_sym);
	       doing_relookup = false;
	       if (assoc_item) {
		  add_item(assoc_item, sym);	// add "ref" symbol for faster lookup
	       }
	    }
	 }
	 else {
	    add_item(&doing_relookup, sym);	// flag as known to short-circuit lookups
	 }
      }
   }
   if (is_base_rep && assoc_item == &doing_relookup) {
      assoc_item = NULL;	// known foreign symbol
   }
   return assoc_item;
}

/*
   START-LOG-------------------------------------------

   $Log: xrefMapper.h.cxx  $
   Revision 1.4 2000/07/10 23:01:44EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
// Revision 1.2  1993/05/17  17:32:30  wmm
// Performance tuning, etc., for subsystem extraction.
//
// Revision 1.1  1993/05/10  11:35:43  wmm
// Initial revision
//
   END-LOG---------------------------------------------
*/

