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
// file cmd_cfm.C
//
// Group of functions for capturing arguments to store them in
// the memory Command Journal.
//
//
// History:  08/07/91    M.Furman   Initial coding for types appTree*,
//                                  objOperType, appTree
//           08/13/91    M.Furman   Reconstruction according to
//                                  capturing preliminary spec and
//                                  addition the new functions
//           08/15/91    M.Furman   Function CC_rel_type added
//           08/19/91    M.Furman   Clean up    

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <genError.h>
#include <cmd_internal.h>

#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <strstream>
#endif /* ISO_CPP_HEADERS */

// Capture header:
// convert header (app *) to (objPlacement) 
objPlacement *CC_hdr(app *hdr)
  {
    if(!hdr)
       return NULL;
    objPlacement *t = new objPlacement;
    CC_hdr(hdr, t);
    return t;
  }

void CC_hdr(app *hdr, objPlacement *t)
    {
    Initialize(CC_hdr);
    if(hdr == NULL)
      {
      t->tree_type = (appType)0;
      t->tree_name = 0;
      }
    else
      {
      t->tree_type = (appType)hdr->get_type();// obtain tree type and
      t->tree_name = hdr->get_name();
      }
    }

// Capture application object:
// convert "appTree *" to "objPlacement" 

objPlacement *CC_node(appTree *x)
   {
    if(!x)
      return NULL;

    objPlacement *t = new objPlacement;
    CC_node(x, t);
    return t;
  }

void CC_node(appTree *x, objPlacement *t)
    {
    objTree *p, *w;
    int i;

    Initialize(CC_node);
    IF(!x) return;
    CC_hdr(x->get_header(), t);

      // Find node position in tree
      for(i = 0, p = x; p != NULL; p = p->get_parent())
        {
        t->tree_path.insert_first((Obj *)i);
        i = 0;
        for(w = p; w != NULL; w = w->get_prev())
          {
          i++;
	  }
        }
  }

// Capture a selection:
// convert objArr of objTree to objArr of objPlacement
objArr *CC_sel(objArr* sel)
{
    objArr *a = new objArr;
    CC_sel(sel, a);
    return a;
}

void CC_sel(objArr* sel, objArr*a)
    {
    Obj *p;
    objPlacement *op;

    Initialize(CC_sel);
    ForEach(p, *sel)	// for all nodes in selection
      {
      op = CC_node(checked_cast(appTree,p));
      a->insert_last((Obj *)op);
      }
    }

// Capture an relation type
// convert "RelType *" to "char *"
char *CC_rel_type(RelType *r)
    {
    Initialize(CC_rel_type);
    ReturnValue(r->get_name());
    }

/*
  $Log: cmd_cfm.C  $
  Revision 1.5 2000/07/10 23:00:58EDT ktrans 
  mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.4  1994/07/20  20:15:04  mg
 * Bug track: NA
 * restructured for logging
 *
*/
