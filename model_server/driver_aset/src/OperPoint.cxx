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
// OperPoint::OperPoint
//------------------------------------------
// synopsis:
// Constructor for operation point
//
// description:
// Operation point (a.k.a. "insertion point")
// Defines the place where an operation is to be performed.
//------------------------------------------
// Restrictions:
// The void* vw pointer is a place the driver
// stores a temporary view pointer. It is a
// void* to keep other modules from needing
// to include view things.
//------------------------------------------

// include files

#include "OperPoint.h"
#include "ste_report.h"

//------------------------------------------
// Constructor OperPoint
//------------------------------------------

OperPoint::OperPoint( symbolPtr nd, objOperType tp, int sub_tp, 
    int xx, int yy, void* vw_nd, const symbolPtr &subnd, int ofset) 
:    type (tp),
     subtype (sub_tp),
     x (xx),
     y (yy),
     view_node (vw_nd),
     offset_of (ofset)
{
    node = nd;
    subnode = subnd;
    window_start = 0;
}

OperPoint::~OperPoint(){} //cfront cannot inline it.

OperPoint::OperPoint() :
     type (FIRST),
     subtype (0),
     x (0),
     y (0),
     view_node (0),
     offset_of (0),
     node((Obj *)0),
     subnode((Obj *)0),
     window_start (0)
{ }

OperPoint::OperPoint(viewPosition &pos) :
     type (FIRST),
     subtype (0),
     x (0),
     y (0),
     view_node (0),
     offset_of (pos.offset),
     node(pos.node),
     subnode((Obj *)0),
     window_start (0)
{ }

/*
   START-LOG-------------------------------------------

   $Log: OperPoint.cxx  $
   Revision 1.4 1996/05/17 16:26:55EDT aharlap 
   Bug track: N/A
   changed symbolPtr argument in OperPoint constructor to const symbolPtr&
Revision 1.2.1.7  1993/07/28  21:12:48  mg
OperPoint::~OperPoint(){}

Revision 1.2.1.6  1993/05/18  13:22:04  boris
Added empty and viewPostion constructors

Revision 1.2.1.5  1992/12/22  20:10:35  aharlap
changed for to do gcc compile this file

Revision 1.2.1.4  1992/12/21  01:52:52  aharlap
changed to use symbolPtr

Revision 1.2.1.3  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:42:31  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
