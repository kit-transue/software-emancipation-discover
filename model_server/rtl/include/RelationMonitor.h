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
#ifndef _RelationMonitor_h
#define _RelationMonitor_h

//------------------------------------------
// RelationMonitor
//------------------------------------------

#ifndef _RTL_h
#include "RTL.h"
#endif

RelClass(RelationMonitor);

#ifdef __GNUG__
    typedef RelType* RelTypePtr;
    typedef RelTypePtr*const RelTypeRef;
    typedef RelTypeRef* RelTypeTable;
#endif

class RelationMonitor : public RTL
{
  public:

#ifndef __GNUG__
    typedef RelType* RelTypePtr;
    typedef RelTypePtr*const RelTypeRef;
    typedef RelTypeRef* RelTypeTable;
#endif

    RelationMonitor(RelationalPtr, RelTypeTable monitors, const int num);
    RelationMonitor(const RelationMonitor&);
    ~RelationMonitor();

    define_relational(RelationMonitor,RTL);
    copy_member(RelationMonitor);

    bool monitors(const RelTypePtr) const;
    int  num_monitors() const { return num_rel_types; }

/*
    virtual void notify_softassoc_insert
                             (objInserter *, objInserter *) {}
    
    virtual void notify_softassoc_remove
                             (objRemover *, objRemover *) {}
    
    virtual void notify_softassoc_merge
                             (objMerger *, objMerger *) {}
    
    virtual void notify_softassoc_split
                             (objSplitter *, objSplitter *) {}
*/
  private:
    const RelTypeTable rel_type_table;
    const int num_rel_types;

    void init(RelationalPtr);
    RelationalPtr obj_of_this() const;
};

generate_descriptor(RelationMonitor,RTL);

/*
   START-LOG-------------------------------------------

   $Log: RelationMonitor.h  $
   Revision 1.1 1993/07/28 19:53:07EDT builder 
   made from unix file
 * Revision 1.2.1.2  1992/10/09  19:25:57  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/
#endif // _RelationMonitor_h
