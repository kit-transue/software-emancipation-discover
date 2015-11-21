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
// ldrDCHierarchy
//------------------------------------------
// Synopsis:
// ldr header for Data Charts
//
// Description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#ifndef _ldrDCHierarchy_h_
#define _ldrDCHierarchy_h_

#include <objRelation.h>
#include <OperPoint.h>
#include <ldrSelection.h>
#include <ldrOODT.h>
#include <ddict.h>
#include <viewNode.h>
#include <symbolArr.h>

// This is the ldr header for Data Charts

class ldrDCHierarchy : public ldrOODT , public viewClass_mixin {
public:
   ldrDCHierarchy(const symbolArr&);
   virtual void insert_obj(objInserter *oi, objInserter *ni);
   virtual void  remove_obj(objRemover *, objRemover * nr);
#ifdef __GNUG__
   ldrDCHierarchy(const ldrDCHierarchy &ll): ldrOODT(ll) { }
#endif
   copy_member(ldrDCHierarchy);
   define_relational(ldrDCHierarchy,ldrOODT);
   void add(symbolPtr);
   void remove_struct(symbolPtr); // remove an item from the DC
   void filter_builtin_relations(int);
   int builtin_relations_filter() const { return builtin_relation_filter; }
   void rebuild_root();
   void import_all();

private:
   int builtin_relation_filter;
   virtual objTreePtr find_ldr(objTree *);
   void do_refresh(const symbolArr&, const symbolArr&);
};

generate_descriptor(ldrDCHierarchy,ldrOODT);

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrDCHierarchy.h  $
   Revision 1.4 1998/08/10 18:21:05EDT pero 
   port to VC 5.0: removal of typename, or, etc.
// Revision 1.6  1993/08/30  21:45:33  wmm
// Fix bug 4621.
//
// Revision 1.5  1993/08/05  23:20:16  wmm
// Fix bug 4185 (allow deferred parsing from OODT views).
//
// Revision 1.4  1993/07/12  18:33:27  aharlap
// put  ifdef __GNUG__ around copy-constructor
//
// Revision 1.3  1993/02/05  16:26:35  wmm
// Support "import and show details" functionality.
//
// Revision 1.2  1993/01/22  22:48:07  wmm
// Support XREF-based ERDs and DCs.
//
// Revision 1.1  1992/12/17  21:30:40  wmm
// Initial revision
//

   END-LOG---------------------------------------------

*/
