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
#ifndef _ldrSmtHierarchy_h_
#define _ldrSmtHierarchy_h_

#include "objRelation.h"
#include "OperPoint.h"
#include "smt.h"
#include "ldrSelection.h"
#include "ldrHierarchy.h"
#include <symbolPtr.h>
// This is the ldr header for Source Mode Tree
// -  hierarchical diagram.

class ldrSmtHierarchy : public ldrHierarchy {

 public:
  ldrSmtHierarchy(smtHeaderPtr, smtTreePtr);
#ifdef __GNUG__
  ldrSmtHierarchy(const ldrSmtHierarchy& r) { *this = r; }
#endif
  virtual void insert_obj(objInserter*, objInserter *);
  virtual void remove_obj(objRemover *, objRemover *);
  virtual void assign_obj(objAssigner *, objAssigner *);
  copy_member(ldrSmtHierarchy);
  define_relational(ldrSmtHierarchy,ldrHierarchy);
  ldrTree * generate_tree(appTree *, int = 0);
 private:
  symbolPtr symbol;

  };

enum ldrSmtMode {full, first, null};

class ldrSmtHierarchyNode : public ldrHierarchyNode
  {
public:
  enum ldrSmtMode string_mode;
  ldrSmtHierarchyNode(ldrSymbolType t = ldrSOMETHING) : ldrHierarchyNode(t) {};
  virtual void send_string(ostream& ostream) const;
  virtual objArr * get_tok_list(int mode = 0);
  };

generate_descriptor(ldrSmtHierarchy,ldrHierarchy);
generate_descriptor(ldrSmtHierarchyNode,ldrHierarchyNode);

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrSmtHierarchy.h  $
   Revision 1.2 1998/04/21 13:21:37EDT aharlap 
   Reviewed by: mg \n Test log: /net/oxen/vol02/tmp/ah.tmp/test_outdir/test_outdir/test.log \n Update for els
 * Revision 1.2.1.2  1992/10/09  19:50:31  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
