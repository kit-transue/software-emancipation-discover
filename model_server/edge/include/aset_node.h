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
#ifndef _ASET_NODE_H
#define _ASET_NODE_H

// ASET_NODE.H
//------------------------------------------
// synopsis:   aset_node.h class.
//
// description:   "aset_node" class based on EDGE's "node" class;
//                it is created to use EDGE layout algorithms
//                while having aset nodes, egdes, and graphics.
//                aset_node has a pointer to aset viewSymbolNode.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
// include files

#include "node.h"

#define  DUMMY_CONNECT_POINT  1

class viewSymbolNode;

class aset_node : public node {

public:
   viewSymbolNode *view_node;      // source of aset_node

   int            dummy_type;      
   int            processed_flag;  // 1 - if processed, 0 - otherwise

// constructors, public methods

public:
   aset_node( viewSymbolNode *);
   virtual void action() {};	 

/*  NOT USED ANYWHERE   ?

    void setcolor (char *col ) { } 
    void setbordercolor (char *col) { }
    void setanchorpoints (char* aa) { }
    void setfontname (char* fn) { }
*/

};

/*
   START-LOG-------------------------------------------

   $Log: aset_node.h  $
   Revision 1.1 1993/04/19 14:05:06EDT builder 
   made from unix file
 * Revision 1.2.1.3  1993/04/19  18:06:52  sergey
 * Commented out unused setcolor, setfontname, etc. methods. Part of bug #3420.
 *
 * Revision 1.2.1.2  1992/10/09  19:03:16  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
