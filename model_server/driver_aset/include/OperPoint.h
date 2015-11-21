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
#ifndef __OperPoint_h
#define __OperPoint_h
// OperPoint::OperPoint
//------------------------------------------
// synopsis:
// Operation point class (a.k.a. "insertion point")
//
// description:
// Defines the place where an operation is to be performed.
//------------------------------------------
// Restrictions:
// The void* vw pointer is a place the driver
// stores a temporary view pointer. It is a
// void* to keep other modules from needing
// to include view things.
//------------------------------------------

// include files

#ifndef _objOper_h
#include <objOper.h>
#endif

#ifndef _symbolPtr_h_
#include <symbolPtr.h>
#endif 
//#ifndef _ste_report_h      
//#include <ste_report.h>
//#endif      
#ifndef NULL
#define NULL 0
#endif
// Referenced classes
RelClass(appTree);
//
// OperPoint
//
class OperPoint : public Obj
{
 public:
    symbolPtr  node;      // Ptr to application node.
    objOperType type;      // e.g., after, first, etc.
    int         subtype;   // Unused for now
    int         x;         // Coords of selection (if any)
    int         y;
    void*       view_node; // Temp. place holder for view ptr
    symbolPtr   subnode;   // Ptr to subnode of application node
    int         offset_of; // offset in the subnode if any, node otherwise
    
    int         window_start; // Window start point

    OperPoint(symbolPtr, objOperType, int subtype = 0, int x=0, int
	      y=0, void* vw_nd = NULL, const symbolPtr& subnode = NULL_symbolPtr,
	      int offset_of = 0);

    OperPoint();
    ~OperPoint();
    //    OperPoint (viewPosition&);

    OperPoint(const OperPoint&);
    OperPoint& operator=(const OperPoint&);
};

//
// inline methods
//
inline OperPoint::OperPoint(const OperPoint& oper_pt)
    : node (oper_pt.node),
      type (oper_pt.type),
      subtype (oper_pt.subtype),
      x (oper_pt.x),
      y (oper_pt.y),
      view_node (oper_pt.view_node),
      subnode (oper_pt.subnode),
      offset_of (oper_pt.offset_of)
{}

inline OperPoint&
OperPoint::operator=(const OperPoint& oper_pt)
{
    node      = oper_pt.node;
    type      = oper_pt.type;
    subtype   = oper_pt.subtype;
    x         = oper_pt.x;
    y         = oper_pt.y;
    view_node = oper_pt.view_node;
    subnode   = oper_pt.subnode;
    offset_of = oper_pt.offset_of;

    return *this;
}

/*
   START-LOG-------------------------------------------

   $Log: OperPoint.h  $
   Revision 1.6 1996/05/17 16:26:36EDT aharlap 
   Bug track: N/A
   changed symbolPtr argument in OperPoint constructor to const symbolPtr&
 * Revision 1.2.1.6  1994/06/07  20:16:25  bhowmik
 * Bug track: .
 * Changed #include syntax to avoid multiple includes
 *
 * Revision 1.2.1.5  1993/07/28  21:13:21  mg
 * OperPoint::~OperPoint(){}
 *
 * Revision 1.2.1.4  1993/05/18  13:21:00  boris
 * Added empty and viewPostion constructors
 *
 * Revision 1.2.1.3  1992/12/21  01:51:51  aharlap
 * changed to use symbolPtr
 *
 * Revision 1.2.1.2  1992/10/09  18:52:45  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
