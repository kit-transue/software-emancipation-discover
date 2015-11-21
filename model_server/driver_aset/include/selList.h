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
#ifndef __selList_h
#define __selList_h
//
// selList.h
//
// Selection List
//
// Maintains the list of currently selected objects
//
// History:
//   07/29/91    F. Barrett    Original
//

#ifndef _objDb_h
#include <objDb.h>
#endif
#ifndef __OperPoint_h      
#include <OperPoint.h>
#endif     
#ifndef _objArr_h
#include <objArr.h>
#endif      

class selectionList
{
 public:
    objArr oper_pts;

    selectionList();

    void push_pt(appTree*, objOperType, int subtype=0, int x=0, int y=0);
    void push_pt(OperPoint&);
    void remove_pt(OperPoint&);
    void remove_all();

    OperPoint* top_pt();
    OperPoint* pop_pt();

    appTree*   top_node();
    appTree*   pop_node();
};


inline void
selectionList::push_pt(appTree* tr, objOperType tp, int subtype, int x, int y)
{
    OperPoint* pt = db_new( OperPoint, (tr, tp, subtype, x, y) );

    oper_pts.insert_first( pt );
}

inline void
selectionList::push_pt(OperPoint& oper_pt)
{
    oper_pts.insert_first( (Obj*)&oper_pt );
}

inline
OperPoint* selectionList::top_pt()
{
   if (oper_pts.empty()) {
      return (OperPoint*) NULL;
   } else {
      return (OperPoint*) oper_pts[0];
   }
}

inline
OperPoint* selectionList::pop_pt()
{
   if (oper_pts.empty()) {
      return (OperPoint*) NULL;
   } else {
      OperPoint* pt = (OperPoint*) oper_pts[0];
      oper_pts.remove(pt);
      return pt;
   }
}

inline void
selectionList::remove_pt(OperPoint& oper_pt)
{
    oper_pts.remove( (Obj*)&oper_pt );
}

/*
   START-LOG-------------------------------------------

   $Log: selList.h  $
   Revision 1.2 1995/08/26 09:38:23EDT wmm 
   New group/subsystem implementation
 * Revision 1.2.1.2  1992/10/09  18:53:20  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
