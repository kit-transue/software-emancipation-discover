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
#ifndef __ldrRTLHeader_h
#define __ldrRTLHeader_h

#include "RTL.h"
#include "ldrHierarchy.h"
#include "ldrSelection.h"
#include "ldrRTLDisplayType.h"

RelClass (ldrRTLHierarchy);

class ldrRTLHierarchy : public ldrHierarchy {

   virtual int dummy_ldrRTLHierarchy ();
 public:
   ldrRTLHierarchy() {};
   ~ldrRTLHierarchy();

   ldrRTLHierarchy (RTLPtr rtl, int sort_order, ldrRTLDisplayType d_type);
#ifdef __GNUG__
   ldrRTLHierarchy(const ldrRTLHierarchy &ll) { *this = ll;}
#endif
   virtual void build_selection(const ldrSelection&, OperPoint&);

   virtual void insert_obj(objInserter *, objInserter *);
   virtual void remove_obj(objRemover *, objRemover *);
   virtual void hilite_obj(objHiliter *, objHiliter *);
   virtual void focus_obj(objFocuser *, objFocuser *);

   virtual void send_string(ostream& stream) const;

   define_relational(ldrRTLHierarchy,ldrHierarchy);
   copy_member(ldrRTLHierarchy);
};

generate_descriptor(ldrRTLHierarchy, ldrHierarchy);

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrRTLHierarchy.h  $
   Revision 1.3 1997/05/21 12:11:16EDT twight 
   Bug track: 14168
   Reviewed by: mb
   
   Changed parameter from 'ldrSelection& const' to 'const ldrSelection&'.
 * Revision 1.2.1.3  1993/03/18  20:28:05  aharlap
 * cleanup for paraset compatibility
 *
 * Revision 1.2.1.2  1992/10/09  19:50:26  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
