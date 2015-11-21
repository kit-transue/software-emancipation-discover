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
#ifndef _symbolLdrHeader_h_
#define _symbolLdrHeader_h_

#include "objRelation.h"
#include "ldrHeader.h"
#include "xref.h"

#define MAX_LEVEL_DEPTH 3

class symbolPtr;
class symbolLdrTree;

RelClass (symbolLdrHeader);

class symbolLdrHeader : public ldrHeader {
   ddKind   obj_type;
   linkType child_lt;
   linkType parent_lt;
   int 	    default_no_levels;
   int      disp_arg;               // 0 - no arguments display is a default for C++

 public:

   symbolLdrHeader(RTLPtr, ddKind, linkType, linkType, int level = MAX_LEVEL_DEPTH);
   symbolLdrHeader(RTLPtr);
   symbolLdrHeader(appPtr app_header, appTreePtr sub_tree = 0);
   symbolLdrHeader(smtHeaderPtr smt_header, smtTreePtr sub_tree = 0);
   symbolLdrHeader(const symbolLdrHeader &);
   void build_selection(const ldrSelection&, OperPoint& app_point);
   virtual void insert_obj(objInserter*, objInserter *);
   virtual void remove_obj(objRemover *, objRemover *);
   define_relational(symbolLdrHeader,ldrHeader);
   virtual void assign_obj(objAssigner*, objAssigner*);
   int get_default_no_levels () const { return default_no_levels; } 
   void set_default_no_levels (int ll); 
   linkType get_parent_lt () const { return parent_lt; }
   linkType get_child_lt () const { return child_lt; }
   ddKind get_obj_type () const { return obj_type; }
   void modify_names();                                 // strip or add arguments
   void set_disp_arg(int ii) { disp_arg = ii; }
   int get_disp_arg() { return disp_arg; }

 private:
   void redo_insert(objInserter*);
};

generate_descriptor(symbolLdrHeader,ldrHeader);


#endif

/*
   START-LOG-------------------------------------------



$Log: symbolLdrHeader.h  $
Revision 1.3 1997/05/21 12:13:06EDT twight 
Bug track: 14168
Reviewed by: mb

Changed parameter from 'ldrSelection& const' to 'const ldrSelection&'.
// Revision 1.5  1993/05/23  21:15:56  sergey
// Just changed a comment about disp_arg default.
//
// Revision 1.4  1993/04/13  23:43:12  sergey
// Added support of hiding arguments in C++ call tree. Part of bug #3305.
//
// Revision 1.3  1993/03/05  21:50:26  sergey
// Added set_default_no_levels method. Part of bug #2684.
//

   END-LOG---------------------------------------------

*/
