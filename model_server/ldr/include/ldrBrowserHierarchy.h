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
#ifndef _ldrBrowserHierarchy_h_
#define _ldrBrowserHierarchy_h_

#ifndef _objRelation_h
#include <objRelation.h>
#endif
#ifndef __OperPoint_h
#include <OperPoint.h>
#endif

#ifndef _ldrSelection_h_
#include <ldrSelection.h>
#endif

#ifndef _ldrOODT_h
#include <ldrOODT.h>
#endif

#ifndef _objArr_h
#include <objArr.h>
#endif

#include <ldrNode.h>

// This is the ldr header for the OODT class browser (aka "inheritance view").

class ldrBrowserHierarchy : public ldrOODT {
public:
   ldrBrowserHierarchy(symbolPtr tr);
   virtual void insert_obj(objInserter *oi, objInserter *ni);
   virtual void  remove_obj(objRemover *, objRemover * nr);
   void reroot(symbolPtr);
   void rebuild_root();
#ifdef __GNUG__
   ldrBrowserHierarchy(const ldrBrowserHierarchy &ll) { *this = ll;}
#endif
// The following copy_member function was causing the compiler to 
// crash, so Alex Harlap and I commented it out.  -jef 08/14/94
//   copy_member(ldrBrowserHierarchy);
   define_relational(ldrBrowserHierarchy,ldrOODT);
   virtual objTreePtr find_ldr(objTree *);
   void import_all();
   void add_inherited_members(ldrClassNodePtr, objArr&);
   void add_relations (ldrClassNodePtr, objArr&);
private:
   void do_refresh(const symbolArr&, const symbolArr&);
};

generate_descriptor(ldrBrowserHierarchy,ldrOODT);

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrBrowserHierarchy.h  $
   Revision 1.7 1998/08/10 18:21:02EDT pero 
   port to VC 5.0: removal of typename, or, etc.
 * Revision 1.1  1994/09/21  18:43:49  jerry
 * Initial revision
 *
 * Revision 1.2.1.7  1994/06/07  20:18:34  bhowmik
 * Bug track: .
 * Changed #include syntax to avoid  duplicate includes
 *
 * Revision 1.2.1.6  1993/08/05  23:20:13  wmm
 * Fix bug 4185 (allow deferred parsing from OODT views).
 *
 * Revision 1.2.1.5  1993/07/02  13:56:10  wmm
 * Fix bug 3810 by only extracting superclass members if requested by the
 * user, and then only for the selected classes.
 *
 * Revision 1.2.1.4  1993/02/05  16:26:35  wmm
 * Support "import and show details" functionality.
 *
 * Revision 1.2.1.3  1993/01/24  01:14:13  wmm
 * Support XREF-based class browser view.
 *
 * Revision 1.2.1.2  1992/10/09  19:50:13  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
