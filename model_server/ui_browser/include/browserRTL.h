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
#ifndef _browserRTL_h
#define _browserRTL_h

// browserRTL.h
//------------------------------------------
// synopsis:
// 
// Specialized RTL that updates lists in projectQuery.
//------------------------------------------

#ifndef _RTL_h
#include <RTL.h>
#endif

#ifndef _ddSelector_h
#include <ddSelector.h>
#endif

#ifndef _symbolScope_h
#include <symbolScope.h>
#endif

class gtRTL;

class browserRTL : public RTL
{
  public:
    browserRTL();
    browserRTL(const symbolArr&);
    browserRTL(const ddSelector&);
    browserRTL(gtRTL *rtl);
    browserRTL(gtRTL *rtl, const symbolArr&);
    browserRTL(gtRTL *rtl, const ddSelector&, symbolScope *scope);
    ~browserRTL();

    define_relational(browserRTL, RTL);
    declare_copy_member(browserRTL);

    void append(const symbolPtr&);
    void append(const symbolArr&);

    virtual void xref_notify(const symbolArr& modified,
			     const symbolArr& added,
			     const symbolArr& deleted);

  private:
    ddSelector  dd_selector;
    gtRTL       *parent_rtl;
    symbolScope *scope;
};

generate_descriptor(browserRTL, RTL);


/*
   START-LOG-------------------------------------------

   $Log: browserRTL.h  $
   Revision 1.2 1995/03/09 12:11:46EST azaparov 
   Bug track: N/A
   Project scope & PARAPARTITION
// Revision 1.2  1993/09/28  20:51:35  azaparov
// Bug track: 4752
// Bug 4752 fixed
//
// Revision 1.1  1993/02/05  05:01:53  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _browserRTL_h
