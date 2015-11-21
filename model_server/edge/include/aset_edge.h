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
#ifndef _ASET_EDGE_H
#define _ASET_EDGE_H

// ASET_NODE.H
//------------------------------------------
// synopsis:   aset_edge.h class.
//
// description:  "aset_edge" class based on EDGE's "edge" class;
//               aset_edge has a pointer to aset viewConnectionNode.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
// include files

#include "edge.h"
#include "general.h"

class viewConnectionNode;

class aset_edge : public edge {

public:

// if this edge is a part of multiple input/output view connector
// it always connects a dummy node and a node from multiple group

   int                mult_index;        // index in multiple group
                                         // -1 - for regular edge

   viewConnectionNode *view_conn;        // source; aset connector


// constructors, public methods

   aset_edge( viewConnectionNode *vv) : edge(NULL) {
        view_conn = vv;
        mult_index = -1;
   }

   virtual void action() {};	 

  void settypename(char* )  {  }
  void setlabel(char* ) {  }
  void setsourcename(char* ) {  }  
  void settargetname(char* ) { }
  void setfontname (char* ) { }
  void setarrowcolor (char *) { }
  void setcolor (char *) { }    

};
/*
   START-LOG-------------------------------------------

   $Log: aset_edge.h  $
   Revision 1.2 1995/07/27 20:20:26EDT rajan 
   Port
 * Revision 1.2.1.4  1993/04/22  18:56:52  sergey
 * Clean up. Part of bug #3463.
 *
 * Revision 1.2.1.3  1993/04/19  18:15:46  sergey
 * Minor clean up to suppress the warning. Pat of bug #3420.
 *
 * Revision 1.2.1.2  1992/10/09  19:03:10  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
