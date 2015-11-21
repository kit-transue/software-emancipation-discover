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
#ifndef _ldrOODT_h
#define _ldrOODT_h

// ldrOODT.h
//------------------------------------------
// synopsis:
//
// ldr header for ERD, Data Chart, and Inheritance views.  Provides support
// for deferred reparse in graphical views.
//------------------------------------------

#include <ldrHierarchy.h>

#ifndef _objArr_h
#include <objArr.h>
#endif

#ifndef _symbolArr_h
#include <symbolArr.h>
#endif

RelClass(smtHeader);
RelClass(RTLNode);

class ldrOODT: public ldrHierarchy {
public:
   define_relational(ldrOODT,ldrHierarchy);

   ldrOODT();
   ~ldrOODT();

   virtual void rebuild_root() = 0;
   
   void add_pending(smtHeaderPtr);
   bool has_pending();
   void refresh_pending();
   void parse_pending();

   static objSet modifiedObjectsByPropagation;
   static objSet* get_modifiedOBP();
   static int doNotRefreshPropagated(ldrOODT *);
   static void refreshModifiedOBP();

   static void refresh_after_get(const symbolArr& before, const symbolArr& after);
    
private:
   virtual void do_refresh(const symbolArr&, const symbolArr&) = 0;
   RTLNodePtr pending;
   static objArr OODT_headers;
};

generate_descriptor(ldrOODT,ldrHierarchy);

/*----------------------------------------------------------------------
  $Log: ldrOODT.h  $
  Revision 1.2 1995/10/26 21:29:03EDT wmm 
  Bug track: 10077,11055
  Also fix unscriptable bugs 10381 (situation cannot arise after these changes), 11009 (requires WM actions, not available in scripts), and
  10078 (can't script to check for nonexistence of UDR connector).
// Revision 1.3  1993/09/22  01:21:11  pero
// Bug track: 4776
// modify to update view at the end of propagation (class def of ldrOODT and
// viewGraHeader)
//
// Revision 1.2  1993/08/30  21:45:44  wmm
// Fix bug 4621.
//
// Revision 1.1  1993/08/05  23:20:27  wmm
// Initial revision
//
*/
#endif
