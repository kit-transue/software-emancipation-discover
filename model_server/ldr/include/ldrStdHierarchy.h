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
#ifndef _ldrStdHierarchy_h_
#define _ldrStdHierarchy_h_

#include "objRelation.h"
#include "objOper.h"
#include "ldrHierarchy.h"
#include "steDocument.h"
#include "steTextNode.h"
#include "ldrNode.h"

// This is the ldr header for abbriviated hierarchical
// representation of Structured Text Documents (STD).
// Abbreviated representation means that it is shows
// only structured` nodes (sections, chapters), but
// does not shoe paragraphs.

class ldrStdHierarchy : public ldrHierarchy
{
 protected:
   ldrStdHierarchy() { }
 public:
   ldrStdHierarchy(steDocumentPtr Doc,
		   steTextNodePtr text_root = (steTextNodePtr)NULL,
		   boolean build_full_hierarchy = false);
#ifdef __GNUG__
   ldrStdHierarchy(const ldrStdHierarchy &ll) { *this = ll;}
#endif
   virtual void insert_obj(objInserter*, objInserter *);
   virtual void remove_obj(objRemover *, objRemover *);
   copy_member(ldrStdHierarchy);
   define_relational(ldrStdHierarchy,ldrHierarchy);
 protected:
   steTextNodePtr find_visible_ancestor(steTextNodePtr);
   virtual ldrHierarchyNodePtr extract_hierarchy(steTextNodePtr);
   virtual boolean is_visible_node(steTextNodePtr);
   virtual void fix_insertion_source(objInserter* the_old,
				     objInserter* the_new);
   virtual void fix_insertion_target(objInserter* the_old,
				     objInserter* the_new);
};

generate_descriptor(ldrStdHierarchy,ldrHierarchy);


// Ldr Header for Full hierarchical diagrams of
// Structured Text Documents -- shows both structured
// and non-structured nodes (paragraphs)

class ldrStdFullHierarchy : public ldrStdHierarchy
{
 protected:
   ldrStdFullHierarchy() { }
 public:
   ldrStdFullHierarchy(steDocumentPtr Doc,
		       steTextNodePtr text_root = (steTextNodePtr)NULL);
#ifdef __GNUG__
   ldrStdFullHierarchy(const ldrStdFullHierarchy &ll) { *this = ll;}
#endif
   copy_member(ldrStdFullHierarchy);
   define_relational(ldrStdFullHierarchy,ldrStdHierarchy);
   /* override */ void build_selection(const ldrSelection&, OperPoint&);
 protected:
   virtual ldrHierarchyNodePtr extract_hierarchy(steTextNodePtr);
   virtual boolean is_visible_node(steTextNodePtr);
   /* override */ void fix_insertion_source(objInserter* the_old,
                                            objInserter* the_new);
   virtual void fix_insertion_target(objInserter* the_old,
				     objInserter* the_new);
};

generate_descriptor(ldrStdFullHierarchy,ldrStdHierarchy);

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrStdHierarchy.h  $
   Revision 1.3 1997/05/22 09:43:38EDT twight 
   Changed '& const' into 'const &'.
 * Revision 1.2.1.3  1992/11/30  16:44:01  smit
 * include ldrNode.h to fix syntax errors.
 *
 * Revision 1.2.1.2  1992/10/09  19:50:35  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
