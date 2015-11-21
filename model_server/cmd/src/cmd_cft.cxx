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
// cmd_cft.C
// Group of functions to make Functional Text Representation from 
// the memory Command Journal values to store it in
// the Command Journal file
//
//
// History:  08/07/91    M.Furman   Initial coding for types appTree*,
//                                  objOperType, appTree
//           08/13/91    M.Furman   Reconstruction according to
//                                  capturing preliminary spec and
//                                  addition the new functions
//           08/15/91    M.Furman   Function CT_rel_type added
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

// Convert "objOperType" to "char"
char *CT_oper_type(objOperType t)
    {
    Initialize(CT_oper_type);
    switch(t)
      {
      case NULLOP:
        return("NULLOP"       );
      case FIRST: 
        return("FIRST"        );
      case AFTER: 
        return("AFTER"        );
      case REPLACE:
        return("REPLACE"      );
      case REPLACE_RIGHT: 
        return("REPLACE_RIGHT");
      case CHANGE_NAME:
        return("CHANGE_NAME");
      }
    Assert(0);	// can't get here
    return NULL;
    }

// Convert objPlacement to text for header
char *CT_hdr(objPlacement *p)
{
    if(!p) return NULL;
    ostrstream s;
    CT_hdr(p, s);
    return s.str();
  }

void CT_hdr(objPlacement *p, ostream& s)
    {
      Initialize(CT_hdr);
      Assert(p);
      s << "C_hdr("
        << p->tree_type
        << ",\"" 
        << p->tree_name  
        << "\")";
    }

// Convert objPlacement to text for application object
char *CT_node(objPlacement *p)
    {
    if(p == NULL)
       return NULL;

    ostrstream s;
    CT_node(p, s);
    return s.str();
  }

void CT_node(objPlacement *p, ostream& s)
    {
    Obj *fw;
    int w;

    Initialize(CT_node);
    Assert(p);

      s << "C_node("
        << p->tree_type  
        << ",\"" 
        << p->tree_name
        << "\"";
      ForEach(fw, p->tree_path)
        {
        w = (int)fw;
        s << "," 
          << w;
        } 
      s << ",0)";
    }

// Convert selection to text
char *CT_sel(objArr* sel)
  {
    ostrstream s;
    CT_sel(sel, s);
    return s.str();
  }

void CT_sel(objArr* sel, ostream& s)
    {
    Obj *op;
    int first = 0;
    char *w;

    Initialize(CT_sel);
    s << "app_sel(";
    ForEach(op, *sel)
      {
      if(first++)
        s << ",";
      w = CT_node((objPlacement *)op);
      s << w;
      delete w;
      }    
    s << ")";
    }

// Convert relation type identifier to text
char *CT_rel_type(char *t)
    {
    ostrstream s;
    CT_rel_type(t, s);
    return s.str();
  }

void CT_rel_type(char *t, ostream& s)
    {
    Initialize(CT_rel_type);	
    s << "C_rel_type(\"" 
      << t 
      << "\")";
    }

