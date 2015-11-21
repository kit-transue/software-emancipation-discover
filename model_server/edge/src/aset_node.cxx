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
// aset_node::aset_node
//------------------------------------------
// synopsis: aset_node constructor.
//
// description: creates aset_node object from the input view_node.
//              Sets back pointers  to the view source node.
//
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files


#include "all.h"
#include "globdef.h"

#include "EDGEAset.h"
#include "viewEdge.h"

#include "aset_node.h"

#include "genError.h"
#include "objTree.h"
#include "objOper.h"
#include "ldrNode.h"
#include "viewNode.h"
#include "viewGraHeader.h"


//------------  aset_node constructor ----------------------

   aset_node::aset_node( viewSymbolNodePtr vv) : node((char *)NULL) {
    Initialize(aset_node::aset_node);

    float sym_width, sym_height;
    float cx, cy;
    int   lev;

    if (vv != NULL){
       vv->get_symbol_size(cx, cy, sym_width, sym_height, 0);
       setwidth((int) sym_width);
       setheight((int) sym_height);

#ifdef HORIZ_TREE
       viewGraHeaderPtr vh = checked_cast(viewGraHeader,vv->get_header());
       int vtype = (vh) ? vh->get_type() : Rep_ERD;
       if (vtype == Rep_Tree || vtype == Rep_OODT_Inheritance || vtype ==
             Rep_OODT_Relations || vtype == Rep_OODT_Scope) {
	  setwidth((int) sym_height);
	  setheight((int) sym_width);
       }
#endif
       view_node = vv;               // set back ptr to view node
       lev = vv->get_graphlevel();
       setlevel(lev);
    }
    else{                            // dummy
       setwidth((int) 1);
       setheight((int) 1);
       view_node = NULL;
    }
    dummy_type = 0;               // for multiple nodes connector
    processed_flag = 0;
 
  }

/*
   START-LOG-------------------------------------------

   $Log: aset_node.cxx  $
   Revision 1.1 1992/11/21 16:27:05EST builder 
   made from unix file
Revision 1.2.1.3  1992/11/21  21:26:40  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:57:35  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
