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
#if 0
#ifndef _ldrProjHierarchy_h_
#define _ldrProjHierarchy_h_

#include "objRelation.h"
#include "OperPoint.h"
#include "projectHeader.h"
#include "projectNode.h"
#include "ldrSelection.h"
#include "ldrHierarchy.h"

// This is the ldr header for Project Tree
// -  hierarchical diagram.

class ldrProjHierarchy : public ldrHierarchy {

 public:
  ldrProjHierarchy(projectHeaderPtr, projectNodePtr);
#ifdef __GNUG__
  ldrProjHierarchy(const ldrProjHierarchy& r) { *this = r; }
#endif
  virtual void insert_obj(objInserter*, objInserter *);
  virtual void remove_obj(objRemover *, objRemover *);
  copy_member(ldrProjHierarchy);
  define_relational(ldrProjHierarchy,ldrHierarchy);
  ldrTree * generate_tree(appTree *, int = 0);
  };

class ldrProjHierarchyNode : public ldrHierarchyNode
  {
private:
  int show_modules_flag : 1;

public:
  ldrProjHierarchyNode(ldrSymbolType t = ldrSOMETHING) : ldrHierarchyNode(t) {};
  void show_modules (int show_flag);
  virtual void send_string(ostream& ostream) const;
  virtual objArr * get_tok_list(int mode = 0);
  define_relational(ldrProjHierarchy,ldrHierarchy);

  int get_show_flag ();
  void set_show_flag (int flag);
  };

generate_descriptor(ldrProjHierarchy,ldrHierarchy);
generate_descriptor(ldrProjHierarchyNode,ldrHierarchyNode);

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrProjHierarchy.h  $
   Revision 1.2 1994/11/14 19:07:24EST mg 
   Bug track: NA
   virtual int get_symbolPtr()
 * Revision 1.2.1.3  1993/04/23  17:23:32  davea
 * bug 3482 - specify return types for get_show_flag and set_show_flag
 *
 * Revision 1.2.1.2  1992/10/09  19:50:23  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
#endif
