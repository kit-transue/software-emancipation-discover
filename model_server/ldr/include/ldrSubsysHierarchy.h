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
#ifndef _ldrSubsysHierarchy_h_
#define _ldrSubsysHierarchy_h_

#include "objRelation.h"
#include "OperPoint.h"
#include "ldrSelection.h"
#include "ldrOODT.h"

// This is the ldr header for the OODT subsystem browser.

class ldrSubsysHierarchy : public ldrOODT {
public:
   define_relational(ldrSubsysHierarchy,ldrOODT);
   copy_member(ldrSubsysHierarchy);
   ldrSubsysHierarchy(appTreePtr tr);

   ldrSubsysHierarchy(const ldrSubsysHierarchy &ll): ldrOODT(ll) { }

   virtual void insert_obj(objInserter *oi, objInserter *ni);
   virtual void  remove_obj(objRemover *, objRemover * nr);
   void reroot(appTreePtr);
   void rebuild_root();
   void import_all();
private:
   void do_refresh(const symbolArr&, const symbolArr&);
};

generate_descriptor(ldrSubsysHierarchy,ldrOODT);

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrSubsysHierarchy.h  $
   Revision 1.3 1998/08/10 18:21:13EDT pero 
   port to VC 5.0: removal of typename, or, etc.
 * Revision 1.2.1.7  1993/08/30  21:45:48  wmm
 * Fix bug 4621.
 *
 * Revision 1.2.1.6  1993/07/13  12:09:35  builder
 * *** empty log message ***
 *
 * Revision 1.2.1.5  1993/07/12  18:41:14  aharlap
 * put #ifdef __GNUG__ around copy-constr
 *
 * Revision 1.2.1.4  1993/02/05  16:26:35  wmm
 * Support "import and show details" functionality.
 *
 * Revision 1.2.1.3  1993/01/25  23:58:34  wmm
 * Support XREF-based subsystem views.
 *
 * Revision 1.2.1.2  1992/10/09  19:50:36  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
