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
// file cmd_cfu.C
// 
// Group of functions to find/make arguments from its memory Command 
// Journal representation (for UNDO) 
//
// History:  08/07/91    M.Furman   Initial coding for types appTree*,
//                                  objOperType, appTree
//           08/13/91    M.Furman   Reconstruction according to
//                                  capturing preliminary spec and
//                                  addition the new functions
//           08/15/91    M.Furman   Function CU_rel_type added
//           08/19/91    M.Furman   Clean up

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "genError.h"
#include "cmd_internal.h"
#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <strstream>
#endif /* ISO_CPP_HEADERS */


// Find header from objPlacement
app *CU_hdr(objPlacement *op)
    {
    Initialize(CU_hdr);
    ReturnValue(checked_cast(app,app::get_header(op->tree_type, op->tree_name)));
    }

// Find app object from objPlacement 
appTree *CU_node(objPlacement *op)
    {
    app *hdr;
    appTree *p;
    int a, i;
    Obj *fa;

    Initialize(CU_node);
    hdr = checked_cast(app,app::get_header(op->tree_type, op->tree_name));
    if(hdr == NULL)
      ReturnValue(NULL);
    p = checked_cast(appTree,hdr->get_root());          // p = root node address
    ForEach(fa, op->tree_path)
      {
      a = (int)fa;
      p = checked_cast(appTree,p->get_first());
      if(p == NULL)
        break;
      for(i = 1; i < a; i++)
        {
        p = checked_cast(appTree,p->get_next());
        if(p == NULL) break;
        }
      }
    ReturnValue(p);
    }

// Make selection array from array of objPlacement
objArr *CU_sel(objArr* obj)
    {
    Obj *op;
    appTree *p;
    objArr *sel = new objArr;

    Initialize(CU_sel);
    ForEach(op, *obj)
      {
      p = CU_node((objPlacement *)op);
      sel->insert_last(p);
      }
    ReturnValue(sel);
    }

// Make pointer for relation type descriptor from identifier
RelType *CU_rel_type(char *id)
    {
    Initialize(CU_rel_type);
    ReturnValue(C_rel_type(id));
    }












