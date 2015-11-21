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
#ifndef __ldrRTLNode_h_
#define __ldrRTLNode_h_

#include "objRelation.h"
#include "OperPoint.h"
#include "RTL.h"
#include "ldrNode.h"
#include "ldrRTLDisplayType.h"
#include <genString.h>

RelClass (ldrRTLNode);

class ldrRTLNode : public ldrHierarchyNode {
 private:
  int    display_flag;   // what to display
  int    ascending;      // sorting order
  objString format_spec; // field format spec
  objString sort_spec;   // field sort spec
  symbolPtr obj;            // Object itself;
  genString old_name;    // Old name for Hierarhicals and SymbolPtr
 public:
   ldrRTLNode() {};
#ifdef __GNUG__
   ldrRTLNode(const ldrRTLNode &ll) { *this = ll;}
#endif
   void set_display_flag (int flag) {display_flag = flag;}
   int  get_display_flag () const { return display_flag;}
   void set_obj (symbolPtr& p_obj) {obj = p_obj;}
   const symbolPtr& get_obj () const { return obj;}
   void set_ascending (int flag) {ascending = flag;}
   int get_ascending () { return ascending;}

   virtual void send_string(ostream& stream) const;
   virtual void print(ostream& /*const*/=cout, int level=0) const;

   declare_copy_member(ldrRTLNode);
   define_relational(ldrRTLNode,ldrHierarchyNode);
   int is_rename_occurred ();
   int set_old_name();
  
   virtual int get_symbolPtr(symbolPtr*) const;

};

generate_descriptor(ldrRTLNode, ldrHierarchyNode);

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrRTLNode.h  $
   Revision 1.4 1994/11/18 00:04:39EST builder 
   
 * Revision 1.2.1.6  1993/06/09  22:25:17  boris
 * Added old_name field and rename checking
 *
 * Revision 1.2.1.5  1993/02/17  22:08:45  kol
 * commented const (in arg for print)
 *
 * Revision 1.2.1.4  1992/12/22  23:48:28  aharlap
 * *** empty log message ***
 *
 * Revision 1.2.1.3  1992/12/09  20:22:51  aharlap
 * added interface for new xref
 *
 * Revision 1.2.1.2  92/10/09  19:50:27  jon
 * RCS History Marker Fixup
 * 


   END-LOG---------------------------------------------

*/
