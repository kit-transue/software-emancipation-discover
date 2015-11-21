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
// selectionList
//------------------------------------------
// synopsis:
// Impliments selection stack
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files

#include "selList.h"

//------------------------------------------
// Constructor selectionList
//------------------------------------------

selectionList::selectionList()
 : oper_pts ()
{
}


//------------------------------------------
// remove_all
//
// Remove all points from the selection list
//------------------------------------------

void selectionList::remove_all()
{
    Initialize (selectionList::remove_all);

    oper_pts.removeAll();

    Return;
}

//------------------------------------------
// top_node
//
// Return the node of the top selected point
//------------------------------------------

appTree* selectionList::top_node()
{
   Initialize (selectionList::top_node);

   OperPoint* pt = this->top_pt ();


   if (pt == NULL) {
      ReturnValue (NULL);
   } else {
      ReturnValue (pt->node);
   }
}

//------------------------------------------
// pop_node
//
// Return the node of the top selected point and pop it
//------------------------------------------

appTree* selectionList::pop_node()
{
   OperPoint* pt = this->top_pt();

   Initialize (selectionList::pop_node);

   if (pt == NULL) {
      ReturnValue (NULL);
   } else {
      oper_pts.remove((Obj*)pt);
      ReturnValue (pt->node);
   }
}
/*
   START-LOG-------------------------------------------

   $Log: selList.cxx  $
   Revision 1.1 1992/11/21 16:05:24EST builder 
   made from unix file
Revision 1.2.1.3  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:44:23  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
